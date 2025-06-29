using Google.Protobuf.RedisProtocol;
using Microsoft.Extensions.Caching.Memory;

namespace WS_SB.Services
{
    public class LocalCacheService
    {
        private readonly IMemoryCache   _memoryCache;

        /* 캐시 키 */

        private const string            LOCAL_REGISTER_PREFIX = "local_log::";
        private const string            SERVER_KEY = "servers";

        /* 캐시 타이머 */

        private const int               LOCAL_ID_LOG_EXPIRE_MIN = 10;

        public LocalCacheService(IMemoryCache memoryCache)
        {
            _memoryCache = memoryCache;
        }

        public void SetServerList(List<R_SERVER_DATA> serverList, TimeSpan expireTime)
        {
            _memoryCache.Set(SERVER_KEY, serverList, expireTime);
        }

        public List<R_SERVER_DATA>? GetServerList()
        {
            return _memoryCache.Get<List<R_SERVER_DATA>>(SERVER_KEY);
        }

        public void SetLocalRegisterLog(string name, string? password)
        {
            _memoryCache.Set(LOCAL_REGISTER_PREFIX + name, password, TimeSpan.FromMinutes(LOCAL_ID_LOG_EXPIRE_MIN));
        }

        public bool GetLocalRegisterLog(string name, out string? password)
        {
            bool isExisted = _memoryCache.TryGetValue(LOCAL_REGISTER_PREFIX + name, out password);
            if (isExisted)
            {
                SetLocalRegisterLog(name, password); 
            }
            return isExisted;
        }
    }
}
