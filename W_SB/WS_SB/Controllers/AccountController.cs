using Google.Protobuf.Struct;
using Google.Protobuf.HttpProtocol;
using Google.Protobuf.RedisProtocol;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using System.Web;
using WS_SB.Services;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;

namespace WS_SB.Controllers
{
    [Route("[controller]")]
    [ApiController]
    public class AccountController : BaseProtobufController
    {
        /* Dependency Injection */

        private readonly AccountService _accountService;

        public AccountController(AccountService accountService)
        {
            _accountService = accountService;
        }

        /* Http 함수 */

        [HttpPost("ReqCheckExistsAccount")]
        public override async Task<IActionResult> HandleReqCheckExistsAccount([FromBody] REQ_CHECK_EXISTS_ACCOUNT pkt)
        {
            var res = new RES_CHECK_EXISTS_ACCOUNT();

            /* 패킷 검증 */

            if (_accountService.ValidateLocalSignUpInfo(pkt.AccountName) == false)
            {
                res.Success = false;
                return Ok(res);
            }

            res.Success = !_accountService.IsExistLocalAccount(pkt.AccountName);
            return Ok(res);
        }

        [HttpPost("ReqCreateAccount")]
        public override async Task<IActionResult> HandleReqCreateAccount([FromBody] REQ_CREATE_ACCOUNT pkt)
        {
            var res = new RES_CREATE_ACCOUNT();

            /* 패킷 검증 */

            if (_accountService.ValidateLocalSignUpInfo(pkt.AccountName, pkt.Password) == false)
            {
                res.Success = false;
                return Ok(res);
            }

            res.Success = await _accountService.CreateLocalAccount(pkt.AccountName, pkt.Password);
            return Ok(res);
        }

        [HttpPost("ReqLoginAccount")]
        public override async Task<IActionResult> HandleReqLoginAccount([FromBody] REQ_LOGIN_ACCOUNT pkt)
        {
            RES_LOGIN_ACCOUNT res;

            /* 패킷 검증 */

            if (_accountService.ValidateLocalSignUpInfo(pkt.AccountName, pkt.Password) == false)
            {
                res = new RES_LOGIN_ACCOUNT();
                res.Success = false;
                return Ok(res);
            }

            /* 계정 DB 조회 */

            res = _accountService.LoginLocalAccount(pkt.AccountName, pkt.Password);
            return Ok(res);
        }

        [HttpPost("ReqLoginGoogleAccount")]
        public override async Task<IActionResult> HandleReqLoginGoogleAccount([FromBody] REQ_LOGIN_GOOGLE_ACCOUNT pkt)
        {
            RES_LOGIN_ACCOUNT res;
            string? authCode = HttpUtility.UrlDecode(pkt.AuthCode);
            if (authCode == null)
            {
                res = new RES_LOGIN_ACCOUNT();
                res.Success = false;
                return Ok(res);
            }

            /* 구글 계정 조회 */

            res = await _accountService.LoginGoogleAccount(authCode);
            return Ok(res);
        }

        [Authorize]
        [HttpPost("ReqConnectGameServer")]
        public override async Task<IActionResult> HandleReqConnectGameServer([FromBody] REQ_CONNECT_GAME_SERVER pkt)
        {
            var res = new RES_CONNECT_GAME_SERVER();
            if (pkt.AccountId.ToString() == User.Claims.FirstOrDefault(claim => claim.Type == ClaimTypes.NameIdentifier)?.Value)
            {
                R_SERVER_DATA? serverData = await _accountService.ConnectGameServer(pkt.AccountId, pkt.ServerId);
                if (serverData != null)
                {
                    res.ServerInfo = new ServerInfo
                    {
                        ServerId = serverData.ServerId,
                        Name = serverData.Name,
                        Density = serverData.Density,
                        IpAddress = serverData.IpAddress,
                        Port = serverData.Port
                    };
                    res.Success = true;
                    return Ok(res);
                }
            }
            res.Success = false;
            return Ok(res);
        }

        [Authorize]
        [HttpPost("ReqRecheckServer")]
        public override async Task<IActionResult> HandleReqRecheckServer([FromBody] REQ_RECHECK_SERVER pkt)
        {
            var res = new RES_RECHECK_SERVER();

            if (pkt.AccountId.ToString() == User.Claims.FirstOrDefault(claim => claim.Type == ClaimTypes.NameIdentifier)?.Value)
            {
                List<R_SERVER_DATA>? serverList = _accountService.GetServerList();
                foreach (R_SERVER_DATA server in serverList ?? [])
                {
                    res.ServerList.Add(new ServerSelectInfo
                    {
                        ServerId = server.ServerId,
                        Name = server.Name,
                        Density = server.Density
                    });
                }
                res.Success = true;
                return Ok(res);
            }
            res.Success = false;
            return Ok(res);
        }
    }
}
