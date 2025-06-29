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
	DBEnterTaskExecutor
**************************/

const int32 DBEnterTaskExecutor::EXIT_ACCOUNT_TTL_MIN = 15;

DBEnterTaskExecutor::DBEnterTaskExecutor()
{
	_jwtPassword = GetJwtPasswordStr();

	char accountKeyPrefix[256];
	Utils::UTF16To8(L"account::", OUT accountKeyPrefix);
	_accountKeyPrefix = xString(accountKeyPrefix);

	char playingAccountKey[256];
	Utils::UTF16To8(L"playing_accounts", OUT playingAccountKey);
	_playingAccountKey = xString(playingAccountKey);
}

DBEnterTaskExecutor::~DBEnterTaskExecutor()
{
}

void DBEnterTaskExecutor::GetVerifiedAccount(GameSessionRef gameSession, int32 accountId, xString tokenValue)
{
	/* JWT 토큰 검사 */

	{
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
			/* 인증 실패 */

			GConsoleLogger->WriteStdOut(Color::YELLOW, L"JWT token is invalid : %ls", Utils::UTF8To16(err.what()).c_str());
			{
				Protocol::S_LOGIN loginPkt;
				loginPkt.set_result(Protocol::LOGIN_RESULT_ERROR_INVALID_TOKEN);
				SEND_S_PACKET(gameSession, loginPkt);
			}
			gameSession->Disconnect(L"Disconnection becase of wrong token");
			return;
		}
	}

	/* 웹 서버에서 올린 Redis 데이터 탐색 */

	std::string redisAccountValue = DownloadAccountPlayers(accountId);
	if (redisAccountValue.empty() == true)
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

	/* 인증 성공 */

	Protocol::R_ACCOUNT_DATA accountData;
	ASSERT_CRASH(accountData.ParseFromString(redisAccountValue));


	ConnectionRoomRef connectionRoom = GRoomManager->GetConnectionRoom();
	connectionRoom->DoAsync(&ConnectionRoom::LoadPlayerDatas, gameSession, accountId, xVector<Protocol::R_PLAYER_DATA>(accountData.player_datas().begin(), accountData.player_datas().end()));
	return;
}

void DBEnterTaskExecutor::ClearVerifiedAccount(int32 accountId, Protocol::R_ACCOUNT_DATA accountData)
{
	UploadAccountPlayers(accountId, accountData);
}

const std::string DBEnterTaskExecutor::GetJwtPasswordStr()
{
	std::string jwtPassStr;
	char buffer[256];
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

const std::string DBEnterTaskExecutor::DownloadAccountPlayers(const int32& accountId)
{
	auto redis = GDBManager->GetRedis();
	xString accountIdStr = xString(std::to_string(accountId));
	xString accountKey = _accountKeyPrefix + accountIdStr;

	std::string redisAccountValue = "";
	{
		sw::redis::Transaction transaction = redis->transaction(false, false);
		sw::redis::Redis tmpRedis = transaction.redis();

		/* Redis 트랜잭션 실행 */

		while (true)
		{
			try
			{
				tmpRedis.watch({ accountKey, _playingAccountKey });
				bool isExistedAccount = tmpRedis.sismember(_playingAccountKey, accountIdStr);
				sw::redis::OptionalString res = tmpRedis.get(accountKey);
				if (res.has_value() == true && isExistedAccount == false)
				{
					transaction.sadd(_playingAccountKey, accountIdStr);
					redisAccountValue = res.value();
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
				GConsoleLogger->WriteStdOut(Color::YELLOW, L"Getting account has redis error : %ls", Utils::UTF8To16(err.what()).c_str());
				return std::string();
			}
		}
	}

	return redisAccountValue;
}

void DBEnterTaskExecutor::UploadAccountPlayers(const int32& accountId, const Protocol::R_ACCOUNT_DATA& accountData)
{
	auto redis = GDBManager->GetRedis();

	xString accountIdStr = xString(std::to_string(accountId));
	xString accountKey = _accountKeyPrefix + accountIdStr;

	std::string redisAccountValue = accountData.SerializeAsString();
	{
		sw::redis::Transaction transaction = redis->transaction(false, false);
		
		/* Redis 트랜잭션 실행 */

		try
		{
			transaction
				.set(accountKey, redisAccountValue, std::chrono::minutes(EXIT_ACCOUNT_TTL_MIN))
				.srem(_playingAccountKey, accountIdStr);
			transaction.exec();
		}
		catch (const sw::redis::Error& err)
		{
			GConsoleLogger->WriteStdOut(Color::YELLOW, L"Adding account has redis error : %ls", Utils::UTF8To16(err.what()).c_str());
		}
	}
}

/*************************
	DBUpdateTaskExecutor
**************************/

DBUpdateTaskExecutor::DBUpdateTaskExecutor()
{
	_serverInfo = xnew<Protocol::R_SERVER_DATA>();

	_serverInfo->set_server_id(1ul);
	{
		char u8Name[256];
		Utils::UTF16To8(L"테스트1", u8Name);
		_serverInfo->set_name(u8Name);
	}
	_serverInfo->set_ip_address("127.0.0.1");
	_serverInfo->set_port("7777");

	char serverKey[256];
	Utils::UTF16To8(L"servers", OUT serverKey);
	_serverKey = xString(serverKey);

	char updateAccountKey[256];
	Utils::UTF16To8(L"update_accounts", OUT updateAccountKey);
	_updateAccountKey = xString(updateAccountKey);
}

DBUpdateTaskExecutor::~DBUpdateTaskExecutor()
{
	xdelete(_serverInfo);
	_serverInfo = nullptr;
}

void DBUpdateTaskExecutor::UpdateServerInfo(float dencity)
{
	_serverInfo->set_density(dencity);

	auto redis = GDBManager->GetRedis();
	try
	{
		redis->hset(_serverKey, std::to_string(_serverInfo->server_id()), _serverInfo->SerializeAsString());
	}
	catch (const sw::redis::Error& err)
	{
		GConsoleLogger->WriteStdOut(Color::YELLOW, L"Updating server has Redis error : %ls", Utils::UTF8To16(err.what()).c_str());
	}
}

void DBUpdateTaskExecutor::UpdatePlayer(int32 accountId, Protocol::R_PLAYER_DATA playerData)
{
	auto redis = GDBManager->GetRedis();
	try
	{
		redis->hset(_updateAccountKey, std::to_string(accountId), playerData.SerializeAsString());
	}
	catch (const sw::redis::Error& err)
	{
		GConsoleLogger->WriteStdOut(Color::YELLOW, L"Updating player has Redis error : %ls", Utils::UTF8To16(err.what()).c_str());
	}
}
