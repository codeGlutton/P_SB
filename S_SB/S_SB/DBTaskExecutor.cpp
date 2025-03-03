#include "pch.h"
#include "DBTaskExecutor.h"
#include "DBManager.h"

#include "GameSession.h"
#include "ClientPacketHandler.h"

#include "Room.h"
#include "RoomManager.h"

#include "RedisProtocol.pb.h"
#include <sw/redis++/redis++.h>

#include <jwt-cpp/jwt.h>

#include "GenProcedures.h"
#include "DBConnectionPool.h"

/*************************
	DBPlayerTaskExecutor
**************************/

DBPlayerTaskExecutor::DBPlayerTaskExecutor()
{
	_serverInfo = xnew<Protocol::R_SERVER_DATA>();

	_serverInfo->set_id(1ul);
	{
		char u8Name[256];
		Utils::UTF16To8(L"테스트1", u8Name);
		_serverInfo->set_name(u8Name);
	}
	_serverInfo->set_ip_address("127.0.0.1");
	_serverInfo->set_port("7777");

	_jwtPassword = GetJwtPasswordStr();
}

DBPlayerTaskExecutor::~DBPlayerTaskExecutor()
{
	xdelete(_serverInfo);
	_serverInfo = nullptr;
}

void DBPlayerTaskExecutor::UpdateServerInfo(float dencity)
{
	_serverInfo->set_density(dencity);

	auto redis = GDBManager->GetRedis();

	char redisKey[256];
	Utils::UTF16To8(L"servers", OUT redisKey);
	try
	{
		redis->hset(redisKey, std::to_string(_serverInfo->id()), _serverInfo->SerializeAsString());
	}
	catch (const sw::redis::Error& err)
	{
		GConsoleLogger->WriteStdOut(Color::YELLOW, L"Updating Server has Redis error : %ls", Utils::UTF8To16(err.what()).c_str());
	}
}

void DBPlayerTaskExecutor::GetVerifiedAccount(GameSessionRef gameSession, int32 accountId, xString tokenValue)
{
	bool isSuccess = true;

	/* 웹 서버에서 올린 Redis 데이터 탐색 */

	xString accountIdStr = xString(std::to_string(accountId));

	char accountKey[256];
	Utils::UTF16To8(L"account::", OUT accountKey);
	xString accountKeyStr = xString(accountKey) + accountIdStr;

	char logKey[256];
	Utils::UTF16To8(L"log::", OUT logKey);
	xString logKeyStr = xString(logKey) + accountIdStr;

	auto redis = GDBManager->GetRedis();
	std::string protoStr = "";
	{
		sw::redis::Transaction transaction = redis->transaction(false,false);
		sw::redis::Redis tmpRedis = transaction.redis();

		/* Redis 트랜잭션 실행 */

		while (true)
		{
			try
			{
				tmpRedis.watch(accountKeyStr);

				sw::redis::OptionalString res = tmpRedis.get(accountKeyStr);
				if (res)
				{
					protoStr = res.value();
					transaction.set(logKeyStr, "", std::chrono::minutes(30));
				}
				transaction.exec();
				
				break;
			}
			catch (const sw::redis::WatchError& err)
			{
				// 중도에 타 클라이언트 방해
				continue;
			}
			catch (const sw::redis::Error& err)
			{
				GConsoleLogger->WriteStdOut(Color::YELLOW, L"Players from account has Redis error : %ls", Utils::UTF8To16(err.what()).c_str());
				isSuccess = false;
				break;
			}
		}
	}

	/* JWT 토큰 검사 */

	auto decoded = jwt::decode(tokenValue.c_str());
	auto verifier = jwt::verify()
		.with_subject(std::to_string(accountId))
		.allow_algorithm(jwt::algorithm::hs256{ _jwtPassword.c_str() });
	try
	{
		verifier.verify(decoded);
	}
	catch (const std::exception& err)
	{
		GConsoleLogger->WriteStdOut(Color::YELLOW, L"JWT token is invalid : %ls", Utils::UTF8To16(err.what()).c_str());
		isSuccess = false;
	}

	if (isSuccess == true)
	{
		/* 인증 성공 */

		Protocol::R_ACCOUNT_DATA accountData;
		ASSERT_CRASH(accountData.ParseFromString(protoStr));

		/* Account의 게임 서버 접속 여부 Redis 기록 */

		char setKey[256];
		Utils::UTF16To8(L"in_game::accounts", OUT setKey);

		if (redis->sadd(setKey, accountIdStr) == 1ll)
		{
			ConnectionRoomRef connectionRoom = GRoomManager->GetConnectionRoom();
			connectionRoom->DoAsync(&ConnectionRoom::LoadPlayerDatas, gameSession, accountId, xVector<Protocol::ObjectInfo>(accountData.object_infos().begin(), accountData.object_infos().end()));
			return;
		}
		else
		{
			/* 중복 연결 처리 */

			{
				Protocol::S_LOGIN loginPkt;
				loginPkt.set_result(Protocol::LOGIN_RESULT_ERROR_ACCOUNT_EXIST);
				SEND_S_PACKET(gameSession, loginPkt);
			}
			gameSession->Disconnect(L"Disconnection because the account is already connected");
			return;
		}
	}

	/* 인증 실패 */

	{
		Protocol::S_LOGIN loginPkt;
		loginPkt.set_result(Protocol::LOGIN_RESULT_ERROR_INVALID_TOKEN);
		SEND_S_PACKET(gameSession, loginPkt);
	}
	gameSession->Disconnect(L"Disconnection becase of wrong token");
	//GRoomManager->GetConnectionRoom()->DoTimer(2000ull, ([gameSession] { gameSession->Disconnect(L"Reserved disconnection becase of wrong token"); }));
}

void DBPlayerTaskExecutor::ClearVerifiedAccount(int32 accountId)
{
	xString accountIdStr = xString(std::to_string(accountId));

	auto redis = GDBManager->GetRedis();
	char setKey[256];
	Utils::UTF16To8(L"in_game::accounts", OUT setKey);

	redis->srem(setKey, accountIdStr);
}

const std::string DBPlayerTaskExecutor::GetJwtPasswordStr()
{
	std::string jwtPassStr;
	char buffer[32767];
	DWORD size = GetEnvironmentVariableA("Authentication__JwtPass", buffer, sizeof(buffer));

	if (size > 0)
	{
		jwtPassStr = buffer;
	}
	else
	{
		CRASH("Jwt Env val is empty");
	}

	return jwtPassStr;
}

/*************************
	DBAchvTaskExecutor
**************************/
