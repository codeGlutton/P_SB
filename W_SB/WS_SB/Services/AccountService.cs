using Google.Protobuf;
using Google.Protobuf.Struct;
using Google.Protobuf.RedisProtocol;
using StackExchange.Redis;
using WS_SB.Models;
using WS_SB.DB;
using Microsoft.EntityFrameworkCore;
using Google.Protobuf.WellKnownTypes;
using Google.Protobuf.HttpProtocol;
using System.IdentityModel.Tokens.Jwt;
using Microsoft.Data.SqlClient;

namespace WS_SB.Services
{
    public class AccountService
    {
        /* Services */

        private LocalCacheService               _localCacheService;

        private JwtTokenFactory                 _jwtTokenService;
        private LocalAccountService             _localAccountService;
        private GoogleService                   _googleService;

        private readonly ILogger                _logger;

        /* DB Services */

        private readonly ApplicationDbContext   _context;
        private readonly IConnectionMultiplexer _redis;

        /* Redis 캐시 키 */

        private static readonly string          PLAYING_ACCOUNT_KEY = "playing_accounts";
        private static readonly string          ACCOUNT_PREFIX = "account::";

        /* Redis 캐시 타이머 */

        private static readonly int             ACCOUNT_LOG_EXPIRE_S = 10;

        public enum DBInclude
        {
            None = 0,
            Account = 1,
            Player = 2
        }

        public AccountService(
            LocalCacheService localCacheService,
            JwtTokenFactory jwtTokenService,
            LocalAccountService localAccountService,
            GoogleService googleService,
            ILoggerFactory loggerFactory,
            ApplicationDbContext context,
            IConnectionMultiplexer redis)
        {
            _localCacheService = localCacheService;
            _jwtTokenService = jwtTokenService;
            _localAccountService = localAccountService;
            _googleService = googleService;
            _logger = loggerFactory.CreateLogger("Account");

            _context = context;
            _redis = redis;
        }

        public bool ValidateLocalSignUpInfo(string name)
        {
            return _localAccountService.ValidateAccountName(name);
        }
        public bool ValidateLocalSignUpInfo(string name, string password)
        {
            return _localAccountService.ValidateAccountName(name) && _localAccountService.ValidatePassword(password);
        }
        public bool IsExistLocalAccount(string name)
        {
            /* 로컬 캐시 우선 검사 */

            string? cachedPassword;
            if (_localCacheService.GetLocalRegisterLog(name, out cachedPassword) == true)
            {
                return true;
            }

            /* DB 검사 */

            if (FindLocalRegister(name) != null)
            {
                return true;
            }

            _logger.LogTrace("{0} account was already created", name);
            return false;
        }

        public async Task<bool> CreateLocalAccount(string name, string password)
        {
            /* DB에 이미 존재 시 */

            if (IsExistLocalAccount(name) == true)
            {
                return false;
            }

            /* DB에 없는 신규 계정 시 */

            return await Task.Run(() =>
            {
                using (var transaction = _context.Database.BeginTransaction())
                {
                    while (true)
                    {
                        try
                        {
                            _context.Registers.Add(new RegisterDB()
                            {
                                Name = name,
                                Password = password,
                                Account = new AccountDB()
                                {
                                    AccountSnsType = SnsType.Local,
                                    AccountSnsSub = "Error",
                                    Players = new[]
                                    {
                                    new PlayerDB(),
                                    new PlayerDB(),
                                    new PlayerDB()
                                }
                                }
                            });
                            _context.SaveChanges();

                            RegisterDB? newRegister = _context.Registers
                                                       .Include(row => row.Account)
                                                       .Where(row => row.Name == name)
                                                       .FirstOrDefault();
                            if (newRegister == null)
                            {
                                throw new Exception("Registering local account has error");
                            }
                            newRegister.Account.AccountSnsSub = newRegister.RegisterDBId.ToString();
                            _context.SaveChanges();

                            transaction.Commit();
                            break;
                        }
                        catch (DbUpdateConcurrencyException ex)
                        {
                            // 동시성 충동
                            transaction.Rollback();
                            ex.Entries.Single().Reload();
                            continue;
                        }
                        catch (DbUpdateException ex)
                        {
                            if (ex.InnerException is SqlException sqlEx && sqlEx.Number == 2627)
                            {
                                // Name 중복
                                return false;
                            }
                            throw;
                        }
                        catch
                        {
                            throw;
                        }
                    }
                }

                /* DB 반영 성공 */

                _localCacheService.SetLocalRegisterLog(name, password);
                return true;
            });
        }
        public async Task<AccountDB> FindOrCreateGoogleAccount(string snsSub)
        {
            /* DB에 이미 존재 시 */

            AccountDB? existedAccount = FindAccount(SnsType.Google, snsSub);
            if (existedAccount != null)
            {
                return existedAccount;
            }

            /* DB에 없는 신규 계정 시 */

            return await Task.Run(() =>
            {
                AccountDB? newAccount;
                while (true)
                {
                    try
                    {
                        newAccount = new AccountDB()
                        {
                            AccountSnsType = SnsType.Google,
                            AccountSnsSub = snsSub,
                            Players = new[]
                            {
                            new PlayerDB(),
                            new PlayerDB(),
                            new PlayerDB()
                        }
                        };
                        _context.Accounts.Add(newAccount);
                        if (_context.SaveChangesEx() == true)
                        {
                            break;
                        }
                    }
                    catch
                    {
                        throw;
                    }
                }

                return newAccount;
            });
        }

        public RES_LOGIN_ACCOUNT LoginLocalAccount(string name, string password)
        {
            var loginResult = new RES_LOGIN_ACCOUNT();

            /* 로컬 계정 검사 */

            RegisterDB? localRegister = FindLocalRegister(name, password, DBInclude.Account);
            if (localRegister == null)
            {
                _logger.LogTrace("Invaild login");
                loginResult.Success = false;
                return loginResult;
            }

            /* 토큰 생성 및 서버 정보 갱신 */

            string token = _jwtTokenService.CreateJwtToken(localRegister.Account.AccountDBId);
            loginResult.AccountId = localRegister.Account.AccountDBId;
            loginResult.TokenValue = token;
            foreach (R_SERVER_DATA server in GetServerList() ?? [])
            {
                loginResult.ServerList.Add(new ServerSelectInfo
                {
                    ServerId = server.ServerId,
                    Name = server.Name,
                    Density = server.Density
                });
            }
            loginResult.Success = true;
            return loginResult;
        }
        public async Task<RES_LOGIN_ACCOUNT> LoginGoogleAccount(string authCode)
        {
            var loginResult = new RES_LOGIN_ACCOUNT();

            /* exchange 토큰 검사 */

            GoogleExchangeData? exchangeData = await _googleService.ExchangeTokenAsync(authCode);
            if (exchangeData == null)
            {
                _logger.LogTrace("Invaild google token");
                loginResult.Success = false;
                return loginResult;
            }

            /* google 로그인 계정 생성 혹은 불러오기 */

            JwtSecurityToken jwtSecurityToken = _jwtTokenService.DecipherJwtToken(exchangeData.id_token);
            string snsSub = jwtSecurityToken.Subject;
            if (snsSub == null)
            {
                _logger.LogTrace("Invaild google sub");
                loginResult.Success = false;
                return loginResult;
            }
            AccountDB account = await FindOrCreateGoogleAccount(snsSub);

            /* 토큰 생성 및 서버 정보 갱신 */

            string token = _jwtTokenService.CreateJwtToken(account.AccountDBId);
            loginResult.AccountId = account.AccountDBId;
            loginResult.TokenValue = token;
            foreach (R_SERVER_DATA server in GetServerList() ?? [])
            {
                loginResult.ServerList.Add(new ServerSelectInfo
                {
                    ServerId = server.ServerId,
                    Name = server.Name,
                    Density = server.Density
                });
            }
            loginResult.Success = true;
            return loginResult;
        }

        public async Task<R_SERVER_DATA?> ConnectGameServer(int accountId, int serverId)
        {
            AccountDB? existedAccount = FindAccount(accountId, DBInclude.Player);
            if (existedAccount == null)
            {
                return null;
            }
            if (false == await UploadAccountPlayersToRedis(existedAccount.AccountDBId, existedAccount.Players))
            {
                _logger.LogTrace("Already logged in");
                return null;
            }
            return GetServerList(serverId);
        }

        public List<R_SERVER_DATA>? GetServerList()
        {
            return _localCacheService.GetServerList();
        }
        public R_SERVER_DATA? GetServerList(int id)
        {
            return _localCacheService.GetServerList()?.Find(server => server.ServerId == id);
        }

        /* DB 테이블 Select 함수 */

        public RegisterDB? FindLocalRegister(string name, DBInclude dbInclude = DBInclude.None)
        {
            RegisterDB? register = FindLocalRegister(row => row.Name == name, dbInclude);

            if (register != null)
            {
                _localCacheService.SetLocalRegisterLog(register.Name, register.Password);
            }
            return register;
        }
        public RegisterDB? FindLocalRegister(string name, string password, DBInclude dbInclude = DBInclude.None)
        {
            RegisterDB? register = FindLocalRegister(row => row.Name == name && row.Password == password, dbInclude);

            if (register != null)
            {
                _localCacheService.SetLocalRegisterLog(register.Name, register.Password);
            }
            return register;
        }
        private RegisterDB? FindLocalRegister(Func<RegisterDB, bool> where, DBInclude dbInclude = DBInclude.None)
        {
            RegisterDB? register;
            switch (dbInclude)
            {
                case DBInclude.Account:
                    {
                        register = _context.Registers
                                    .AsNoTracking()
                                    .Include(row => row.Account)
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
                case DBInclude.Player:
                    {
                        register = _context.Registers
                                    .AsNoTracking()
                                    .Include(row => row.Account)
                                        .ThenInclude(row => row.Players)
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
                default:
                    {
                        register = _context.Registers
                                    .AsNoTracking()
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
            }
            return register;
        }

        public AccountDB? FindAccount(int accountId, DBInclude dBInclude = DBInclude.None)
        {
            AccountDB? account = FindAccount(row => row.AccountDBId == accountId, dBInclude);

            return account;
        }
        public AccountDB? FindAccount(SnsType snsType, string snsSub, DBInclude dBInclude = DBInclude.None)
        {
            AccountDB? account = FindAccount(row => row.AccountSnsType == snsType && row.AccountSnsSub == snsSub, dBInclude);

            return account;
        }
        public AccountDB? FindAccount(Func<AccountDB, bool> where, DBInclude dbInclude = DBInclude.None)
        {
            AccountDB? account;
            switch (dbInclude)
            {
                case DBInclude.Player:
                    {
                        account = _context.Accounts
                                    .AsNoTracking()
                                    .Include(row => row.Players)
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
                default:
                    {
                        account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
            }
            return account;
        }

        /* 로컬 함수 */

        /// <summary>
        /// 레디스 캐시에 Account 데이터를 갱신
        /// </summary>
        /// <param name="accountId"> 업로드 대상 계정 id </param>
        /// <param name="players"> 업로드 대상 players </param>
        /// <param name="timeout"> Redis TTL </param>
        /// <returns> 중복 게임 접속 여부 </returns>
        private async Task<bool> UploadAccountPlayersToRedis(int accountId, ICollection<PlayerDB> players)
        {
            return await Task.Run(() =>
            {
                var newRedisData = new R_ACCOUNT_DATA();
                foreach (PlayerDB player in players ?? [])
                {
                    newRedisData.PlayerDatas.Add(new R_PLAYER_DATA
                    {
                        PlayerDbId = player.PlayerDBId,
                        PlayerTableId = player.AssetTableId,
                        Name = player.Name ?? string.Empty,
                        CostumeSetting = Convert.ToUInt32(player.CostumeSetting),
                        RankScore = player.Score,
                        CreatedTime = Timestamp.FromDateTimeOffset(player.CreatedTime)
                    });
                }

                byte[] newRedisBytes = newRedisData.ToByteArray();
                string redisKey = ACCOUNT_PREFIX + accountId.ToString();
                IDatabase db = _redis.GetDatabase();
                while (true)
                {
                    var transaction = db.CreateTransaction();
                    ConditionResult inGameResult = transaction.AddCondition(Condition.SetNotContains(PLAYING_ACCOUNT_KEY, accountId));
                    transaction.StringSetAsync(redisKey, newRedisBytes, expiry: TimeSpan.FromSeconds(ACCOUNT_LOG_EXPIRE_S), when: When.NotExists);
                    if (transaction.Execute() == true)
                    {
                        // 접속 중인 계정 중복 로그인 시도 감지
                        if (inGameResult.WasSatisfied == false)
                        {
                            return false;
                        }
                        return true;
                    }
                }
            });
        }
    }
}
