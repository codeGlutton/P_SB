#include "pch.h"

#include "ThreadManager.h"

#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"

#include "Room.h"
#include "RoomManager.h"

#include "DBConnectionPool.h"
#include "DBSynchronizer.h"
#include "DBManager.h"
#include "GenProcedures.h"

#include "TableRow.h"

#include <sw/redis++/redis++.h>

enum
{
	WORKER_TICK = 64
};

void DoGameWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		// 최대 시간 업데이트
		LEndTickCount = GetTickCount64() + WORKER_TICK;

		/* 네트워크, 로직, 타이머, 글로벌 큐 작업을 구분없이 모든 스레드에서 진행 */

		// 네트워크 패킷 받고, 각 게임 로직 처리 
		service->GetIocpCore()->Dispatch(10);

		// 예약된 일감 (타이머) 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 JobQ 작업도 처리
		ThreadManager::DoGlobalJobQueueWork();
	}
}

void DoDBWorkerJob()
{
	while (true)
	{
		// 최대 시간 업데이트
		LEndTickCount = GetTickCount64() + WORKER_TICK;

		// 글로벌 ProdConsQ 작업도 처리
		ThreadManager::DoGlobalProdConsQueueWork();

		// 글로벌 JobQ 작업도 처리
		ThreadManager::DoGlobalJobQueueWork();
	}
}

int main()
{
	GSessionManager = xnew<GameSessionManager>();

	GDBManager = xnew<DBManager>();
	GDataTable = xnew<DataTable>();

	GRoomManager = xnew<RoomManager>();
	GRoomManager->Init();

	ClientPacketHandler::Init();

#pragma region RedisTestCode
	//try
	//{
	//	//sw::redis::ConnectionOptions connectionOptions;
	//	//connectionOptions.host = "127.0.0.1";  // Required.
	//	//connectionOptions.port = 6379; // Optional. The default port is 6379.
	//	//connectionOptions.password = "TestRedisCpp";   // Optional. No password by default.
	//	//connectionOptions.db = 1;  // Optional. Use the 0th database by default.
	//	//auto redis = sw::redis::Redis(connectionOptions);

	//	std::string redisConnectionStrings;

	//	{
	//		char buffer[32767];
	//		DWORD size = GetEnvironmentVariableA("RedisConnectionStrings", buffer, sizeof(buffer));

	//		if (size > 0) {
	//			redisConnectionStrings = buffer;
	//		}
	//		else
	//		{
	//			CRASH("Redis Env val is empty")
	//		}
	//	}
 //
	//	auto redis = sw::redis::Redis(redisConnectionStrings);

	//	redis.zadd("sorted_set", "p1", 3);
	//	std::cout << redis.zrank("sorted_set", "p1").value() << std::endl;
	//}
	//catch (const sw::redis::Error& err)
	//{
	//	std::cout << err.what() << std::endl;
	//}
#pragma endregion

#pragma region GameDBCode
	// 직접 로컬 서버 연결 && 편집 가능 계정으로 로그인
	//ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={ODBC Driver 17 for SQL Server};Server=DESKTOP-EFO8AOC\\SQLEXPRESS;Database=GameDB;Uid=VsTest;Pwd=lemonalemona;"));

	// LocalHost로 연결 && Trusted_connection으로 윈도우 인증 로그인
	ASSERT_CRASH(GDBConnectionPool->Connect(2, L"Driver={ODBC Driver 17 for SQL Server};Server=localhost;Database=GameDB;Trusted_connection=Yes;"));

	{
		DBConnection* dbConn = GDBConnectionPool->Pop();
		DBSynchronizer dbSync(*dbConn);
		dbSync.Synchronize(L"GameDB.xml");

		// TODO : 캐릭터 생성
		/*{
			WCHAR name[] = L"페이커";
			WCHAR type[] = L"Mage";

			SP::InsertPlayer insertPlayer(*dbConn);
			insertPlayer.In_Name(name);
			insertPlayer.In_Type(type);
			insertPlayer.In_CreateDate(TIMESTAMP_STRUCT{ 1999, 10, 30 });
			insertPlayer.In_X(Utils::GetRandom(1, 100));
			insertPlayer.In_Y(Utils::GetRandom(1, 100));
			insertPlayer.In_Z(Utils::GetRandom(1, 100));
			insertPlayer.Execute();
		}

		{
			WCHAR name[] = L"제우스";
			WCHAR type[] = L"Knight";

			SP::InsertPlayer insertPlayer(*dbConn);
			insertPlayer.In_Name(name);
			insertPlayer.In_Type(type);
			insertPlayer.In_CreateDate(TIMESTAMP_STRUCT{ 2001, 10, 30 });
			insertPlayer.In_X(Utils::GetRandom(1, 100));
			insertPlayer.In_Y(Utils::GetRandom(1, 100));
			insertPlayer.In_Z(Utils::GetRandom(1, 100));
			insertPlayer.Execute();
		}

		{
			WCHAR name[] = L"구마유시";
			WCHAR type[] = L"Archer";

			SP::InsertPlayer insertPlayer(*dbConn);
			insertPlayer.In_Name(name);
			insertPlayer.In_Type(type);
			insertPlayer.In_CreateDate(TIMESTAMP_STRUCT{ 2000, 10, 30 });
			insertPlayer.In_X(Utils::GetRandom(1, 100));
			insertPlayer.In_Y(Utils::GetRandom(1, 100));
			insertPlayer.In_Z(Utils::GetRandom(1, 100));
			insertPlayer.Execute();
		}*/

		GDBConnectionPool->Push(dbConn);
	}
#pragma endregion

	ServerServiceRef service = MakeXShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeXShared<IocpCore>(),
		MakeXShared<GameSession>,
		100
	);

	ASSERT_CRASH(service->Start());

	// 게임 스레드들 일 시작
	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoGameWorkerJob(service);
			});
	}

	// DB 스레드들 일 시작
	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoDBWorkerJob();
			});
	}

	// 메인 스레드도 일 시작
	DoGameWorkerJob(service);

	GThreadManager->Join();
}