using System.Text.RegularExpressions;

namespace WS_SB.Services
{
    public class LocalAccountService
    {
        private readonly ILogger _logger;

        public LocalAccountService(ILoggerFactory loggerFactory)
        {
            _logger = loggerFactory.CreateLogger("LocalAccount");
        }

        public bool ValidateAccountName(string acccountName)
        {
            /* 1~20글자에 특수문자가 없는지 검사 */

            if (Regex.IsMatch(acccountName, @"^[a-zA-Z0-9ㄱ-ㅎㅏ-ㅣ가-힣]{1,20}$") == false)
            {
                _logger.LogTrace("{0} account name is too long or includes symbols", acccountName);
                return false;
            }

            return true;
        }

        public bool ValidatePassword(string password)
        {
            /* 8~20글자에 하나 이상의 영문 대소문자, 숫자, 허용된 특수문자로 이루어졋는지 검사 */

            if (Regex.IsMatch(password, @"^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[~!@#$%^&*_\-+=`|\\:;""',.?\/])[a-zA-Z0-9~!@#$%^&*_\-+=`|\\:;""',.?\/]{8,20}$") == false)
            {
                _logger.LogTrace("password is not usable");
                return false;
            }

            return true;
        }
    }
}
