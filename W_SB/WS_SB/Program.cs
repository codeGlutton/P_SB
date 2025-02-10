using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using StackExchange.Redis;
using System.Text;
using WS_SB.DB;
using WS_SB.Protobuf;

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

/* 콘테이너에 서비스 추가 */

// DB 연동
var sqlConnectionString = builder.Configuration.GetConnectionString("AccountConnection") ?? throw new InvalidOperationException("Connection string 'MSSQL DefaultConnection' not found.");
builder.Services.AddDbContext<ApplicationDbContext>(options =>
    options.UseSqlServer(sqlConnectionString));

// Redis 연동
var redisConnectionString = builder.Configuration.GetConnectionString("CacheConnection") ?? throw new InvalidOperationException("Connection string 'Redis DefaultConnection' not found.");
builder.Services.AddSingleton<IConnectionMultiplexer>(sp => 
    ConnectionMultiplexer.Connect(redisConnectionString));

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

app.UseAuthorization();

app.MapControllers();

app.Run();
