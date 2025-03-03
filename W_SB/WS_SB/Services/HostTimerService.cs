using Google.Protobuf;
using Google.Protobuf.RedisProtocol;
using Google.Protobuf.Struct;
using StackExchange.Redis;

namespace WS_SB.Services
{
    public class HostTimerService : BackgroundService
    {
        private readonly IConnectionMultiplexer _redis;
        private readonly LocalCacheService      _localCacheService;

        /* 캐시 타이머 */

        private const int               SERVER_UPDATE_TICK_S = 10;

        public HostTimerService(IConnectionMultiplexer redis, LocalCacheService localCacheService)
        {
            _redis = redis;
            _localCacheService = localCacheService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var Tasks = new Task[]
            {
                UpdateServerList(stoppingToken)
            };

            await Task.WhenAll(Tasks);
        }

        private async Task UpdateServerList(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                /* Redis에서 갱신된 서버 리스트 추출 */

                List<ServerSelectInfo> serverList = new List<ServerSelectInfo>();
                IDatabase db = _redis.GetDatabase();
                foreach (var server in await db.HashGetAllAsync("servers") ?? [])
                {
                    R_SERVER_DATA serverData = new R_SERVER_DATA();
                    serverData.MergeFrom(server.Value);
                    serverList.Add(new ServerSelectInfo
                    {
                        Id = serverData.Id,
                        Name = serverData.Name,
                        IpAddress = serverData.IpAddress,
                        Port = serverData.Port,
                        Density = serverData.Density
                    });
                }

                // 로컬 캐시에 반영
                _localCacheService.SetServerList(serverList);

                await Task.Delay(TimeSpan.FromSeconds(SERVER_UPDATE_TICK_S), stoppingToken);
            }
        }
    }
}
