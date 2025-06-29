using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Google.Protobuf.HttpProtocol;

namespace WS_SB.Controllers
{
    [Consumes("application/x-protobuf")]
    [Produces("application/x-protobuf")]
    public abstract class BaseProtobufController : ControllerBase
    {

        [HttpPost("ReqCheckExistsAccount")]
        public abstract Task<IActionResult> HandleReqCheckExistsAccount([FromBody] REQ_CHECK_EXISTS_ACCOUNT pkt);

        [HttpPost("ReqCreateAccount")]
        public abstract Task<IActionResult> HandleReqCreateAccount([FromBody] REQ_CREATE_ACCOUNT pkt);

        [HttpPost("ReqLoginAccount")]
        public abstract Task<IActionResult> HandleReqLoginAccount([FromBody] REQ_LOGIN_ACCOUNT pkt);

        [HttpPost("ReqLoginGoogleAccount")]
        public abstract Task<IActionResult> HandleReqLoginGoogleAccount([FromBody] REQ_LOGIN_GOOGLE_ACCOUNT pkt);

        [HttpPost("ReqConnectGameServer")]
        public abstract Task<IActionResult> HandleReqConnectGameServer([FromBody] REQ_CONNECT_GAME_SERVER pkt);

        [HttpPost("ReqRecheckServer")]
        public abstract Task<IActionResult> HandleReqRecheckServer([FromBody] REQ_RECHECK_SERVER pkt);
    }
}