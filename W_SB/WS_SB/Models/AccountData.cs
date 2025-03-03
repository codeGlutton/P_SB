using Google.Protobuf.Struct;

namespace WS_SB.Models
{
    public class AccountLoginData
    {
        public int                      AccountId { get; set; }

        public string                   TokenValue { get; set; }

        public List<ServerSelectInfo>   ServerList { get; set; }
    }
}
