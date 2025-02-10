#include "pch.h"
#include "DBTaskExecutor.h"

#include "Room.h"
#include "RoomManager.h"

#include "GenProcedures.h"
#include "DBConnectionPool.h"

/*************************
	DBPlayerTaskExecutor
**************************/

void DBPlayerTaskExecutor::GetSelectablePlayers(GameSessionRef gameSession, int32 dbId)
{
	xVector<int32> ids;
	xVector<xString> names;
	xVector<TIMESTAMP_STRUCT> dates;

	{
		DBConnection* dbConn = GDBConnectionPool->Pop();
		SP::GetPlayers getPlayers(*dbConn);

		int32 id = 0;
		int32 userId = 0;
		WCHAR name[100];
		TIMESTAMP_STRUCT date;

		getPlayers.In_UserId(dbId);

		getPlayers.Out_Id(OUT id);
		getPlayers.Out_UserId(OUT userId);
		getPlayers.Out_Name(OUT name);
		getPlayers.Out_CreateDate(OUT date);

		getPlayers.Execute();

		_wsetlocale(LC_ALL, L"korean");

		while (getPlayers.Fetch())
		{
			GConsoleLogger->WriteStdOut(Color::BLUE,
				L"DB_ID[%d] DB_UserID[%d] Name[%s] Date[%d/%d/%d]\n", id, userId, name, date.year, date.month, date.day);

			char nameUTF8[256];
			Utils::UTF16To8(name, nameUTF8);

			ids.push_back(id);
			names.push_back(xString(nameUTF8));
			dates.push_back(date);
		}

		GDBConnectionPool->Push(dbConn);
	}

	ConnectionRoomRef connectionRoom = GRoomManager->GetConnectionRoom();
	connectionRoom->DoAsync(&ConnectionRoom::DeliverDBData, gameSession, ids, names, dates);
}

/*************************
	DBAchvTaskExecutor
**************************/
