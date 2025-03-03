using Google.Protobuf;
using Google.Protobuf.Struct;
using Google.Protobuf.RedisProtocol;
using StackExchange.Redis;
using WS_SB.Models;
using WS_SB.DB;
using Microsoft.EntityFrameworkCore;
using Microsoft.Win32;
using System.Linq;

namespace WS_SB.Services
{
    public class AccountService
    {
        /* Services */

        private LocalCacheService               _localCacheService;

        private JwtTokenService                 _jwtTokenService;
        private LocalAccountService             _localAccountService;
        private GoogleService                   _googleService;

        private readonly ILogger                _logger;

        /* DB Services */

        private readonly ApplicationDbContext   _context;
        private readonly IConnectionMultiplexer _redis;

        public enum DBInclude
        {
            None = 0,
            Account = 1,
            User = 2,
            Ranking = 3
        }

        public AccountService(
            LocalCacheService localCacheService,
            JwtTokenService jwtTokenService,
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

        public bool ValidateToken(int accountId, string tokenValue)
        {
            IDatabase db = _redis.GetDatabase();

            // TODO : JWT 토큰 검증
            if (_jwtTokenService.ValidateJwtToken(tokenValue) == false)
            {
                _logger.LogTrace("Invalid Token : {0} ", tokenValue);
                return false;
            }

            /* Redis 검증 */

            var accountValue = db.StringGet("account::" + accountId);
            if (accountValue.HasValue)
            {
                R_ACCOUNT_DATA accountData = new R_ACCOUNT_DATA();
                accountData.MergeFrom(accountValue);

                if (accountData.AccountId == accountId)
                {
                    return true;
                }
            }
            _logger.LogTrace("Invalid account access");
            return false;
        }

        public async Task<bool> IsExistLocalAccount(string name)
        {
            /* Redis 캐시 우선 검사 */

            IDatabase db = _redis.GetDatabase();
            if (await db.KeyExistsAsync("log::" + name) == true)
            {
                return true;
            }

            /* DB 검사 */

            if (FindLocalAccount(name) != null)
            {
                return true;
            }

            _logger.LogTrace("{0} account was already created", name);
            return false;
        }

        public async Task<bool> CreateLocalAccount(string name, string password)
        {
            if (await IsExistLocalAccount(name) == true)
            {
                return false;
            }

            using (var transaction = _context.Database.BeginTransaction())
            {
                while (true)
                {
                    try
                    {
                        var t = _context.Registers.Add(new RegisterDB()
                        {
                            Name = name,
                            Password = password,
                            Account = new AccountDB()
                            {
                                AccountSnsType = SnsType.Local,
                                AccountSnsSub = "Error"
                            }
                        });
                        _context.SaveChanges();

                        RegisterDB? newRegister = _context.Registers
                                                   .Include(row => row.Account)
                                                   .Where(row => row.Name == name)
                                                   .FirstOrDefault();
                        newRegister.Account.AccountSnsSub = newRegister.RegisterDBId.ToString();
                        _context.SaveChanges();

                        transaction.Commit();
                        break;
                    }
                    catch (DbUpdateConcurrencyException ex)
                    {
                        transaction.Rollback();
                        ex.Entries.Single().Reload();
                        continue;
                    }
                }
            }

            /* DB 반영 성공 */

            IDatabase db = _redis.GetDatabase();
            db.StringSetAsync("log::" + name, "", TimeSpan.FromMinutes(30));

            return true;
        }

        public async Task<AccountDB> CreateGoogleAccount(string snsSub)
        {
            AccountDB account = FindAccount(SnsType.Google, snsSub);
            if (account == null)
            {
                do
                {
                    account = new AccountDB()
                    {
                        AccountSnsType = SnsType.Google,
                        AccountSnsSub = snsSub
                    };
                    _context.Accounts.Add(account);
                } while (_context.SaveChangesEx() == false);
            }
            return account;
        }

        public RegisterDB FindLocalAccount(string name, DBInclude dbInclude = DBInclude.None)
        {
            RegisterDB? register =  FindLocalAccount(row => row.Name == name, dbInclude);

            if (register != null)
            {
                IDatabase db = _redis.GetDatabase();
                db.StringSetAsync("log::" + name, "", TimeSpan.FromMinutes(30));
            }

            return register;
        }

        public RegisterDB FindLocalAccount(string name, string password, DBInclude dbInclude = DBInclude.None)
        {
            RegisterDB? register =  FindLocalAccount(row => row.Name == name && row.Password == password, dbInclude);

            if (register != null)
            {
                IDatabase db = _redis.GetDatabase();
                db.StringSetAsync("log::" + name, "", TimeSpan.FromMinutes(30));
            }

            return register;
        }

        private RegisterDB FindLocalAccount(Func<RegisterDB, bool> where, DBInclude dbInclude = DBInclude.None)
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
                case DBInclude.User:
                    {
                        register = _context.Registers
                                    .AsNoTracking()
                                    .Include(row => row.Account)
                                        .ThenInclude(row => row.Users)
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
                case DBInclude.Ranking:
                    {
                        register = _context.Registers
                                    .AsNoTracking()
                                    .Include(row => row.Account)
                                        .ThenInclude(row => row.Users)
                                            .ThenInclude(row => row.Ranking)
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

        public AccountDB FindAccount(SnsType snsType, string snsSub, DBInclude dBInclude = DBInclude.None)
        {
            AccountDB? account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(row => row.AccountSnsType == snsType && row.AccountSnsSub == snsSub)
                                    .FirstOrDefault();
            return account;
        }

        public AccountDB FindAccount(Func<AccountDB, bool> where, DBInclude dbInclude = DBInclude.None)
        {
            AccountDB? account;
            switch (dbInclude)
            {
                case DBInclude.User:
                    {
                        account = _context.Accounts
                                    .AsNoTracking()
                                    .Include(row => row.Users)
                                    .Where(where)
                                    .FirstOrDefault();
                        break;
                    }
                case DBInclude.Ranking:
                    {
                        account = _context.Accounts
                                    .AsNoTracking()
                                    .Include(row => row.Users)
                                        .ThenInclude(row => row.Ranking)
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

        public async Task<List<ServerSelectInfo>> GetServerList()
        {
            return _localCacheService.GetServerList();
        }

        public async Task<AccountLoginData> LoginAccount(string name, string password)
        {
            RegisterDB localAccount = FindLocalAccount(name, password);
            if (localAccount == null)
            {
                _logger.LogTrace("Invaild login");
                return null;
            }
            _context.Entry(localAccount).Reference(row => row.Account).Load();

            AccountLoginData accountLoginData = new AccountLoginData();
            accountLoginData.AccountId = localAccount.Account.AccountDBId;
            accountLoginData.TokenValue = UploadAccountUsers(localAccount.Account);
            accountLoginData.ServerList = await GetServerList(); 

            return accountLoginData;
        }

        public async Task<AccountLoginData> LoginGoogleAccount(string authCode)
        {
            /* exchange 토큰 검사 */

            GoogleExchangeData exchangeData = await _googleService.ExchangeToken(authCode);
            if (exchangeData == null)
            {
                _logger.LogTrace("Invaild google token");
                return null;
            }

            /* google 로그인 계정 생성 혹은 불러오기 */

            var jwtSecurityToken = _jwtTokenService.DecipherJwtToken(exchangeData.id_token);
            string snsSub = jwtSecurityToken.Subject;
            if (snsSub == null)
            {
                _logger.LogTrace("Invaild google sub");
                return null;
            }
            AccountDB account = await CreateGoogleAccount(snsSub);

            /* Redis 업로드 */

            AccountLoginData accountLoginData = new AccountLoginData();
            accountLoginData.AccountId = account.AccountDBId;
            accountLoginData.TokenValue = UploadAccountUsers(account);
            accountLoginData.ServerList = await GetServerList();

            return accountLoginData;
        }

        /* 로컬 함수 */

        private string UploadAccountUsers(AccountDB account)
        {
            string token;
            TimeSpan timeout = _jwtTokenService.CreateJwtToken(account.AccountDBId, out token);

            var enteredAccount = new R_ACCOUNT_DATA { AccountId = account.AccountDBId };
            foreach (var user in account.Users ?? [])
            {
                ObjectInfo objectInfo = new ObjectInfo();
                objectInfo.ObjectBaseInfo = new ObjectBaseInfo();
                objectInfo.ObjectBaseInfo.ObjectId = Convert.ToUInt64(user.AccountDBId);
                objectInfo.PlayerDetailInfo = new PlayerDetailInfo();
                objectInfo.PlayerDetailInfo.Name = user.UserName;
                objectInfo.PlayerDetailInfo.CostumeSetting = Convert.ToUInt32(user.CostumeSetting);

                enteredAccount.ObjectInfos.Add(objectInfo);
            }
            IDatabase db = _redis.GetDatabase();
            db.StringSetAsync("account::" + account.AccountDBId, enteredAccount.ToByteArray(), timeout);

            return token;
        }
    }
}
