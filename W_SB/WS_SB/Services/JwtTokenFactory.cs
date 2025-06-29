using Microsoft.IdentityModel.Tokens;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;

namespace WS_SB.Services
{
    public class JwtTokenFactory
    {
        private readonly string    _jwtPassword;
        private const long          JWT_TIMEOUT_S = 1800; // 30분

        public JwtTokenFactory(IConfiguration config) 
        {
            _jwtPassword = config.GetSection("Authentication")?["JwtPass"] ?? throw new InvalidOperationException("Authentication string 'JwtPass' not found.");
        }

        public string CreateJwtToken(long accountId)
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
            return new JwtSecurityTokenHandler().WriteToken(jwt);
        }

        public JwtSecurityToken DecipherJwtToken(string token)
        {
            var handler = new JwtSecurityTokenHandler();
            return handler.ReadJwtToken(token);
        }
    }
}
