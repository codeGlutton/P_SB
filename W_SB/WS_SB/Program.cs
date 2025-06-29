using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.IdentityModel.Tokens;
using StackExchange.Redis;
using System.Text;
using WS_SB.DB;
using WS_SB.Protobuf;
using WS_SB.Services;

var builder = WebApplication.CreateBuilder(args);

// 로그 설정
if (builder.Environment.IsDevelopment())
{
    builder.Logging.AddConsole();
}
else 
{ 
    builder.Logging.AddFile();
    builder.Logging.AddSeq();
}
builder.Logging.AddConfiguration(builder.Configuration.GetSection("Logging"));

// JWT 인증 설정
var jwtPassword = builder.Configuration.GetSection("Authentication")?["JwtPass"] ?? throw new InvalidOperationException("Authentication string 'JwtPass' not found.");
builder.Services.AddAuthentication(options =>
{
    options.DefaultAuthenticateScheme = JwtBearerDefaults.AuthenticationScheme;
    options.DefaultChallengeScheme = JwtBearerDefaults.AuthenticationScheme;
})
.AddJwtBearer(options =>
{
    options.TokenValidationParameters = new TokenValidationParameters
    {
        ValidateLifetime = true,
        ValidateAudience = false,
        ValidateIssuer = false,
        IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(jwtPassword))
    };
});

/* 콘테이너에 서비스 추가 */

// DB 연동
var sqlConnectionString = builder.Configuration.GetConnectionString("AccountConnection") ?? throw new InvalidOperationException("Connection string 'MSSQL DefaultConnection' not found.");
builder.Services.AddDbContextFactory<ApplicationDbContext>(
        options => options.UseSqlServer(sqlConnectionString));

// Redis 연동
var redisConnectionString = builder.Configuration.GetConnectionString("CacheConnection") ?? throw new InvalidOperationException("Connection string 'Redis DefaultConnection' not found.");
builder.Services.AddSingleton<IConnectionMultiplexer>(sp => 
    ConnectionMultiplexer.Connect(redisConnectionString));

// 호스트 백그라운드
builder.Services.AddMemoryCache();
builder.Services.AddSingleton<LocalCacheService>();
builder.Services.AddHostedService<HostTimerService>();

// 계정 로그인
builder.Services.AddSingleton<JwtTokenFactory>();
builder.Services.AddSingleton<LocalAccountService>();
builder.Services.AddSingleton<GoogleService>();
builder.Services.AddScoped<AccountService>();

// Protobuf 포맷터
builder.Services.AddControllers(options =>
{
    options.InputFormatters.Add(new ProtobufInputFormatter());
    options.OutputFormatters.Add(new ProtobufOutputFormatter());
});

// Learn more about configuring OpenAPI at https://aka.ms/aspnet/openapi
builder.Services.AddOpenApi();
builder.Services.AddEndpointsApiExplorer();

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.MapOpenApi();
}

app.UseHttpsRedirection();

app.UseAuthentication();
app.UseAuthorization();

app.MapControllers();

app.Run();
