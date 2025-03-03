using Microsoft.IdentityModel.Tokens;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;

namespace WS_SB.Services
{
    public class JwtTokenService
    {
        private readonly string     _jwtPassword;
        private const long          JWT_TIMEOUT_S = 1800; // 30분

        public JwtTokenService(IConfiguration config) 
        {
            _jwtPassword = config.GetSection("Authentication")?["JwtPass"];
        }

        public TimeSpan CreateJwtToken(long accountId, out string token)
        {
            long now = new DateTimeOffset(DateTime.UtcNow).ToUnixTimeSeconds();
            long expired = now + JWT_TIMEOUT_S;

            var claims = new Claim[]
            {
                new Claim(JwtRegisteredClaimNames.Sub, accountId.ToString()),
                new Claim(JwtRegisteredClaimNames.Iat, now.ToString(), ClaimValueTypes.Integer64),
                new Claim(JwtRegisteredClaimNames.Exp, expired.ToString(), ClaimValueTypes.Integer64),
            };

            var credentials = new SigningCredentials(
                new SymmetricSecurityKey(Encoding.UTF8.GetBytes(_jwtPassword)),
                SecurityAlgorithms.HmacSha256);

            var jwt = new JwtSecurityToken(claims: claims, signingCredentials: credentials);
            token = new JwtSecurityTokenHandler().WriteToken(jwt);

            return TimeSpan.FromSeconds(JWT_TIMEOUT_S);
        }

        public JwtSecurityToken DecipherJwtToken(string token)
        {
            var handler = new JwtSecurityTokenHandler();
            return handler.ReadJwtToken(token);
        }

        public bool ValidateJwtToken(string token)
        {
            var handler = new JwtSecurityTokenHandler();
            TokenValidationParameters validationParameters = new TokenValidationParameters()
            {
                ValidateLifetime = true,
                ValidateAudience = false,
                ValidateIssuer = false,
                IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(_jwtPassword))
            };

            SecurityToken securityToken;
            try
            {
                var claims = handler.ValidateToken(token, validationParameters, out securityToken);
                return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
