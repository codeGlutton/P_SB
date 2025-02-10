using Google.Protobuf;
using Google.Protobuf.HttpProtocol;
using Google.Protobuf.Protocol;
using Google.Protobuf.Struct;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using StackExchange.Redis;
using System.Net;
using WS_SB.DB;

namespace WS_SB.Controllers
{
    [Route("[controller]")]
    [ApiController]
    public class AccountController : BaseProtobufController
    {
        private readonly ApplicationDbContext _context;

        private readonly ILogger _logger;
        private readonly IConnectionMultiplexer _cache;

        public AccountController(ApplicationDbContext context, ILoggerFactory loggerFactory, IConnectionMultiplexer cache)
        {
            _context = context;
            _logger = loggerFactory.CreateLogger("Account");
            _cache = cache;
        }

        [HttpPost("ReqCheckExistsAccount")]
        public override IActionResult HandleReqCheckExistsAccount([FromBody] REQ_CHECK_EXISTS_ACCOUNT pkt)
        {
            var res = new RES_CHECK_EXISTS_ACCOUNT();

            AccountDB? account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(row => row.AccountName == pkt.AccountName)
                                    .FirstOrDefault();

            if (account == null)
            {
                res.Success = true;
            }
            else
            {
                _logger.LogTrace("{0} account was already created", account.AccountName);
                res.Success = false;
            }

            return Ok(res);
        }

        [HttpPost("ReqCreateAccount")]
        public override IActionResult HandleReqCreateAccount([FromBody] REQ_CREATE_ACCOUNT pkt)
        {
            var res = new RES_CREATE_ACCOUNT();

            AccountDB? account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(row => row.AccountName == pkt.AccountName)
                                    .FirstOrDefault();

            if (account == null)
            {
                _context.Accounts.Add(new AccountDB()
                {
                    AccountName = pkt.AccountName,
                    Password = pkt.Password
                });

                res.Success = _context.SaveChangesEx();
            }
            else
            {
                _logger.LogTrace("{0} account was already created", account.AccountName);
                res.Success = false;
            }

            return Ok(res);
        }

        [HttpPost("ReqLoginAccount")]
        public override IActionResult HandleReqLoginAccount([FromBody] REQ_LOGIN_ACCOUNT pkt)
        {
            var res = new RES_LOGIN_ACCOUNT();

            AccountDB? account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(row => row.AccountName == pkt.AccountName && row.Password == pkt.Password)
                                    .FirstOrDefault();

            if (account == null)
            {
                _logger.LogTrace("Invaild login");
                res.Success = false;
            }
            else
            {
                res.Success = true;

                res.ServerList.Add(new ServerSelectInfo { Name = "TestName", Ip = "127.0.0.1", ClowdedLevel = 0 });
            }

            return Ok(res);
        }
    }
}
