using Google.Protobuf.HttpProtocol;
using Microsoft.AspNetCore.Mvc;
using System.Text;
using System.Web;
using WS_SB.Models;
using WS_SB.Services;

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

            res.Success = !(await _accountService.IsExistLocalAccount(pkt.AccountName));
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
            var res = new RES_LOGIN_ACCOUNT();

            /* 패킷 검증 */

            if (_accountService.ValidateLocalSignUpInfo(pkt.AccountName, pkt.Password) == false)
            {
                res.Success = false;
                return Ok(res);
            }

            /* 계정 DB 비교 */

            AccountLoginData accountLoginData = await _accountService.LoginAccount(pkt.AccountName, pkt.Password);
            if (accountLoginData == null)
            {
                res.Success = false;
                return Ok(res);
            }

            res.AccountId = accountLoginData.AccountId;
            res.TokenValue = accountLoginData.TokenValue;
            if (accountLoginData.ServerList != null)
            {
                res.ServerList.AddRange(accountLoginData.ServerList);
            }
            res.Success = true;
            return Ok(res);
        }

        [HttpPost("ReqLoginGoogleAccount")]
        public override async Task<IActionResult> HandleReqLoginGoogleAccount([FromBody] REQ_LOGIN_GOOGLE_ACCOUNT pkt)
        {
            var res = new RES_LOGIN_ACCOUNT();
            string authCode = HttpUtility.UrlDecode(pkt.AuthCode);

            /* 구글 계정 확인 */

            AccountLoginData accountLoginData = await _accountService.LoginGoogleAccount(authCode);
            if (accountLoginData == null)
            {
                res.Success = false;
                return Ok(res);
            }

            res.AccountId = accountLoginData.AccountId;
            res.TokenValue = accountLoginData.TokenValue;
            if (accountLoginData.ServerList != null)
            {
                res.ServerList.AddRange(accountLoginData.ServerList);
            }
            res.Success = true;
            return Ok(res);
        }

        [HttpPost("ReqRecheckServer")]
        public override async Task<IActionResult> HandleReqRecheckServer([FromBody] REQ_RECHECK_SERVER pkt)
        {
            var res = new RES_RECHECK_SERVER();

            if (_accountService.ValidateToken(pkt.AccountId, pkt.TokenValue) == true)
            {
                var serverList = await _accountService.GetServerList();
                if (serverList != null)
                {
                    res.ServerList.AddRange(serverList); 
                }
                res.Success = true;
                return Ok(res);
            }
            res.Success = false;
            return Ok(res);
        }
    }
}
