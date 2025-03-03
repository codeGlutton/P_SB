using Microsoft.Extensions.Configuration;
using Newtonsoft.Json;
using System.Net.Http;
using System.Net.Http.Headers;
using WS_SB.Models;

namespace WS_SB.Services
{
    public class GoogleService
    {
        private readonly ILogger    _logger;

        HttpClient                  _httpClient;
        private readonly string     _clientId;
        private readonly string     _clientSecret;
        private readonly string     _redirectUrl;

        public GoogleService(ILoggerFactory loggerFactory, IConfiguration config)
        {
            _logger = loggerFactory.CreateLogger("GoogleAccount");
            var googleLgoinConfig = config.GetSection("Authentication")?.GetSection("Google");
            if (googleLgoinConfig != null)
            {
                _clientId = googleLgoinConfig.GetValue<string>("ClientId");
                _clientSecret = googleLgoinConfig.GetValue<string>("ClientSecret");
                _redirectUrl = googleLgoinConfig.GetValue<string>("RedirectUrl");
            }

            _httpClient = new HttpClient() { BaseAddress = new Uri($"https://www.googleapis.com/oauth2/v4/token") };
            _httpClient.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/x-www-form-urlencoded"));
        }

        public async Task<GoogleExchangeData> ExchangeToken(string authCode)
        {
            var reqContent = new FormUrlEncodedContent(new[]
            {
                new KeyValuePair<string, string>("client_id", _clientId),
                new KeyValuePair<string, string>("client_secret", _clientSecret),
                new KeyValuePair<string, string>("code", authCode),
                new KeyValuePair<string, string>("grant_type", "authorization_code"),
                new KeyValuePair<string, string>("redirect_uri", _redirectUrl)
            });
            var res = await _httpClient.PostAsync(_httpClient.BaseAddress, reqContent);
            if (res.IsSuccessStatusCode == false)
            {
                var requestContent = await reqContent.ReadAsStringAsync();
                var responseContent = await res.Content.ReadAsStringAsync();
                _logger.LogTrace("Failed google exchanging. Error code: {0}", res.StatusCode.ToString());
                _logger.LogTrace("{0}", requestContent);
                _logger.LogTrace("{0}", responseContent);
                return null;
            }

            var resContent = await res.Content.ReadAsStringAsync();
            var resData = JsonConvert.DeserializeObject<GoogleExchangeData>(resContent);

            return resData;
        }
    }
}
