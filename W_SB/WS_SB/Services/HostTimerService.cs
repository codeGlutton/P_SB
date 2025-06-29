using EFCore.BulkExtensions;
using Google.Protobuf;
using Google.Protobuf.RedisProtocol;
using Google.Protobuf.Struct;
using Microsoft.EntityFrameworkCore;
using StackExchange.Redis;
using WS_SB.DB;

namespace WS_SB.Services
{
    public class HostTimerService : BackgroundService
    {
        private readonly IDbContextFactory<ApplicationDbContext>    _contextFactory;
        private readonly IConnectionMultiplexer                     _redis;
        private readonly LocalCacheService                          _localCacheService;

        /* 캐시 키 */

        private const string                                        SERVER_UPDATE_KEY = "servers";
        private const string                                        ACCOUNT_UPDATE_KEY = "update_accounts";

        /* 캐시 타이머 */

        private const int                                           SERVER_UPDATE_TICK_S = 10;
        private const int                                           ACCOUNT_UPDATE_TICK_MIN = 10;

        public HostTimerService(IDbContextFactory<ApplicationDbContext> contextFactory, IConnectionMultiplexer redis, LocalCacheService localCacheService)
        {
            _contextFactory = contextFactory;
            _redis = redis;
            _localCacheService = localCacheService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var Tasks = new Task[]
            {
                UpdateServerListAsync(stoppingToken),
                UpdateAccountAsync(stoppingToken)
            };

            await Task.WhenAll(Tasks);
        }

        /// <summary>
        /// 일정 주기마다 게임 서버들의 정보를 redis에서 꺼내 로컬 캐시에 업데이트
        /// </summary>
        private async Task UpdateServerListAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                /* Redis에서 갱신된 서버 리스트 추출 */

                var serverList = new List<R_SERVER_DATA>();
                IDatabase db = _redis.GetDatabase();
                foreach (HashEntry server in db.HashGetAll(SERVER_UPDATE_KEY) ?? [])
                {
                    var serverData = new R_SERVER_DATA();
                    serverData.MergeFrom(server.Value);
                    serverList.Add(serverData);
                }

                // 로컬 캐시에 반영
                _localCacheService.SetServerList(serverList, TimeSpan.FromSeconds(SERVER_UPDATE_TICK_S * 2));

                await Task.Delay(TimeSpan.FromSeconds(SERVER_UPDATE_TICK_S), stoppingToken);
            }
        }

        /// <summary>
        /// Account의 users 정보들을 redis에서 꺼내 일정 주기마다 DB에 업데이트
        /// </summary>
        /// <exception cref="Exception"> Redis에 존재하지 않는 SQL Server user state가 존재 </exception>
        private async Task UpdateAccountAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                var updatePlayers = new List<PlayerDB>();

                IDatabase db = _redis.GetDatabase();
                while (true)
                { 
                    var transaction = db.CreateTransaction();
                    var popAllTask = transaction.HashGetAllAsync(ACCOUNT_UPDATE_KEY);
                    transaction.KeyDeleteAsync(ACCOUNT_UPDATE_KEY);
                    if (await transaction.ExecuteAsync() == true)
                    {
                        foreach (HashEntry updateEntry in await popAllTask ?? [])
                        {
                            var playerData = new R_PLAYER_DATA();
                            playerData.MergeFrom(updateEntry.Value);
                            updatePlayers.Add(new PlayerDB
                            {
                                PlayerDBId = playerData.PlayerDbId,
                                AssetTableId = playerData.PlayerTableId,
                                Name = playerData.Name,
                                CreatedTime = playerData.CreatedTime.ToDateTime(),
                                CostumeSetting = Convert.ToInt32(playerData.CostumeSetting),
                                Score = playerData.RankScore,
                                AccountDBId = Convert.ToInt32(updateEntry.Name)
                            });
                        }
                        break;
                    }
                }

                using (var context = _contextFactory.CreateDbContext())
                {
                    if (updatePlayers.Count > 0)
                    { 
                        await context.BulkUpdateAsync(updatePlayers);
                    }
                }

                await Task.Delay(TimeSpan.FromMinutes(ACCOUNT_UPDATE_TICK_MIN), stoppingToken);
            }
        }
    }
}
