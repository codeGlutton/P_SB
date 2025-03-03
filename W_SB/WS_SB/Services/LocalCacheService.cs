using Google.Protobuf.Struct;

namespace WS_SB.Services
{
    public class LocalCacheService
    {
        private ReaderWriterLockSlim    _serverListlock;
        private List<ServerSelectInfo>  _serverList;

        public LocalCacheService()
        {
            _serverListlock = new ReaderWriterLockSlim();
            _serverList = new List<ServerSelectInfo>();
        }

        public void SetServerList(List<ServerSelectInfo> serverList)
        {
            _serverListlock.EnterWriteLock();
            try
            {
                _serverList = new List<ServerSelectInfo>(serverList);
            }
            finally
            {
                _serverListlock.ExitWriteLock();
            }
        }

        public List<ServerSelectInfo> GetServerList()
        {
            List<ServerSelectInfo> copy;
            _serverListlock.EnterReadLock();
            try
            {
                copy = new List<ServerSelectInfo>(_serverList);
            }
            finally
            {
                _serverListlock.ExitReadLock();
            }
            return copy;
        }
    }
}
