#pragma once
#include "DBConnection.h"
#include "DBModel.h"

/***************************
	   DBSynchronizer
****************************/

class DBSynchronizer
{
	enum
	{
		PROCEDURE_MAX_LEN = 10000
	};

	// 업데이트 순서
	enum UpdateStep : uint8 
	{
		DropIndex,
		AlterColumn,
		AddColumn,
		CreateTable,
		DefaultConstraint,
		CreateIndex,
		DropColumn,
		DropTable,
		StoredProcedure,

		Max
	};

	enum ColumnFlag : uint8
	{
		Type = 1 << 0,
		Nullable = 1 << 1,
		Identity = 1 << 2,
		Default = 1 << 3,
		Length = 1 << 4,
	};

public:
	DBSynchronizer(DBConnection& conn) : _dbConn(conn) { }
	~DBSynchronizer();

	bool							Synchronize(const WCHAR* path);

private:
	void							ParseXmlDB(const WCHAR* path);
	
	bool							GatherDBTables();
	bool							GatherDBIndexes();
	bool							GatherDBStoredProcedures();

	void							CompareDBModel();
	void							CompareTables(DBModel::TableRef dbTable, DBModel::TableRef xmlTable);
	void							CompareColumns(DBModel::TableRef dbTable, DBModel::ColumnRef dbColumn, DBModel::ColumnRef xmlColumn);
	void							CompareStoredProcedures();

	void							ExecuteUpdateQueries();

private:
	DBConnection&					_dbConn;

	xVector<DBModel::TableRef>		_xmlTables;
	xVector<DBModel::ProcedureRef>	_xmlProcedures;
	xSet<xWString>					_xmlRemovedTables;

	xVector<DBModel::TableRef>		_dbTables;
	xVector<DBModel::ProcedureRef>	_dbProcedures;

private:
	// 연관 칼럼이 변경되어 재생성이 필요한 인덱싱
	xSet<xWString>					_dependentIndexes;
	// 동기화를 위해 필요한 쿼리문들
	xVector<xWString>				_updateQueries[UpdateStep::Max];
};