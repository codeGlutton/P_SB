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
        public abstract IActionResult HandleReqCheckExistsAccount([FromBody] REQ_CHECK_EXISTS_ACCOUNT pkt);

        [HttpPost("ReqCreateAccount")]
        public abstract IActionResult HandleReqCreateAccount([FromBody] REQ_CREATE_ACCOUNT pkt);

        [HttpPost("ReqLoginAccount")]
        public abstract IActionResult HandleReqLoginAccount([FromBody] REQ_LOGIN_ACCOUNT pkt);
    }
}