using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Google.Protobuf.HttpProtocol;

namespace WS_SB.Controllers
{
    [Consumes("application/x-protobuf")]
    [Produces("application/x-protobuf")]
    public abstract class BaseProtobufController : ControllerBase
    {
{%- for pkt in parser.recv_pkt %}

        [HttpPost("{{pkt.PascalName}}")]
        public abstract IActionResult Handle{{pkt.PascalName}}([FromBody] {{pkt.name}} pkt);
{%- endfor %}
    }
}
