#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	class InsertPlayer : public DBBind<3, 0>
	{
	public:
		InsertPlayer(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertPlayer(?,?,?)}") { }
		void In_UserId(int32& v) { BindParam(0, v); }
		void In_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); }
		template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(1, v); }
		template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(1, v); }
		void In_Name(WCHAR* v, int32 count) { BindParam(1, v, count); }
		void In_Name(const WCHAR* v, int32 count) { BindParam(1, v, count); }
		void In_CreateDate(TIMESTAMP_STRUCT& v) { BindParam(2, v); }
		void In_CreateDate(TIMESTAMP_STRUCT&& v) { _createDate = std::move(v); BindParam(2, _createDate); }

	private:
		int32 _userId = {};
		TIMESTAMP_STRUCT _createDate = {};
	};

	class GetPlayers : public DBBind<1, 4>
	{
	public:
		GetPlayers(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetPlayers(?)}") { }
		void In_UserId(int32& v) { BindParam(0, v); }
		void In_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); }
		void Out_Id(OUT int32& v) { BindCol(0, v); }
		void Out_UserId(OUT int32& v) { BindCol(1, v); }
		template<int32 N> void Out_Name(OUT WCHAR(&v)[N]) { BindCol(2, v); }
		void Out_CreateDate(OUT TIMESTAMP_STRUCT& v) { BindCol(3, v); }

	private:
		int32 _userId = {};
	};

}