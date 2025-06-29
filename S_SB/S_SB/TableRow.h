#pragma once

enum class TableRowType
{
	NONE = 0,
	UNIFORM_COSTUME_TABLE = 1,
	ATHLETE_TABLE = 1,
	MANAGER_TABLE = 2,
};

/************************
		TableRow
************************/

struct TableRow
{
	int32 Id;
	xWString ServerName;
};

/************************
	CostumeTableRow
************************/

struct CostumeTableRow : public TableRow
{
};

/************************
	SpawnableTableRow
************************/

struct SpawnableTableRow : public TableRow
{
};

/************************
	AthleteTableRow
************************/

struct AthleteTableRow : public SpawnableTableRow
{
	xWString Race;
	int32 Positioin;
	uint8 Skin;
};

/************************
	 DataTableManager
************************/

class DataTableManager
{
public:
	DataTableManager()
	{
		UniformCostumeTable.emplace(std::make_pair(1001, xnew<CostumeTableRow>(CostumeTableRow{1001,L"줄무늬-레드"})));
		CostumeTables.emplace(std::make_pair(TableRowType::UNIFORM_COSTUME_TABLE, &UniformCostumeTable));
		AthleteTable.emplace(std::make_pair(1001, xnew<AthleteTableRow>(AthleteTableRow{1001,L"Human",L"Human",0,0})));
		AthleteTable.emplace(std::make_pair(1002, xnew<AthleteTableRow>(AthleteTableRow{1002,L"Slime",L"Slime",0,0})));
		AthleteTable.emplace(std::make_pair(1003, xnew<AthleteTableRow>(AthleteTableRow{1003,L"Gearon",L"Gearon",0,0})));
		AthleteTable.emplace(std::make_pair(1004, xnew<AthleteTableRow>(AthleteTableRow{1004,L"Roo",L"Roo",0,0})));
		AthleteTable.emplace(std::make_pair(1005, xnew<AthleteTableRow>(AthleteTableRow{1005,L"라이타르 캉게스",L"Roo",1,4})));
		AthleteTable.emplace(std::make_pair(1006, xnew<AthleteTableRow>(AthleteTableRow{1006,L"벨 캉가",L"Roo",1,4})));
		AthleteTable.emplace(std::make_pair(1007, xnew<AthleteTableRow>(AthleteTableRow{1007,L"자르 포샤라",L"Roo",1,3})));
		AthleteTable.emplace(std::make_pair(1008, xnew<AthleteTableRow>(AthleteTableRow{1008,L"쿠엔타르 캉가",L"Roo",3,4})));
		AthleteTable.emplace(std::make_pair(1009, xnew<AthleteTableRow>(AthleteTableRow{1009,L"린타 레프리온",L"Roo",3,1})));
		AthleteTable.emplace(std::make_pair(1010, xnew<AthleteTableRow>(AthleteTableRow{1010,L"벨 스키판",L"Roo",2,2})));
		AthleteTable.emplace(std::make_pair(1011, xnew<AthleteTableRow>(AthleteTableRow{1011,L"크리날 루트하",L"Roo",2,1})));
		AthleteTable.emplace(std::make_pair(1012, xnew<AthleteTableRow>(AthleteTableRow{1012,L"드랙스 호타르",L"Roo",1,3})));
		AthleteTable.emplace(std::make_pair(1013, xnew<AthleteTableRow>(AthleteTableRow{1013,L"프리델 팍스",L"Roo",2,4})));
		AthleteTable.emplace(std::make_pair(1014, xnew<AthleteTableRow>(AthleteTableRow{1014,L"단테 윌리엄스",L"Human",1,2})));
		AthleteTable.emplace(std::make_pair(1015, xnew<AthleteTableRow>(AthleteTableRow{1015,L"이태성",L"Human",1,1})));
		AthleteTable.emplace(std::make_pair(1016, xnew<AthleteTableRow>(AthleteTableRow{1016,L"윤승재",L"Human",2,1})));
		AthleteTable.emplace(std::make_pair(1017, xnew<AthleteTableRow>(AthleteTableRow{1017,L"나카모토 하루야",L"Human",2,1})));
		AthleteTable.emplace(std::make_pair(1018, xnew<AthleteTableRow>(AthleteTableRow{1018,L"장 웨이",L"Human",3,1})));
		AthleteTable.emplace(std::make_pair(1019, xnew<AthleteTableRow>(AthleteTableRow{1019,L"자말 로빈스",L"Human",2,3})));
		AthleteTable.emplace(std::make_pair(1020, xnew<AthleteTableRow>(AthleteTableRow{1020,L"티아고 산토스",L"Human",2,3})));
		AthleteTable.emplace(std::make_pair(1021, xnew<AthleteTableRow>(AthleteTableRow{1021,L"미하일 이바노프",L"Human",2,2})));
		AthleteTable.emplace(std::make_pair(1022, xnew<AthleteTableRow>(AthleteTableRow{1022,L"이반 코박",L"Human",3,2})));
		AthleteTable.emplace(std::make_pair(1023, xnew<AthleteTableRow>(AthleteTableRow{1023,L"잭 카터",L"Human",1,3})));
		AthleteTable.emplace(std::make_pair(1024, xnew<AthleteTableRow>(AthleteTableRow{1024,L"이덴 쿠퍼",L"Human",1,2})));
		AthleteTable.emplace(std::make_pair(1025, xnew<AthleteTableRow>(AthleteTableRow{1025,L"노아 리드",L"Human",1,2})));
		AthleteTable.emplace(std::make_pair(1026, xnew<AthleteTableRow>(AthleteTableRow{1026,L"아니크 로이",L"Human",2,4})));
		AthleteTable.emplace(std::make_pair(1027, xnew<AthleteTableRow>(AthleteTableRow{1027,L"마테오 라미레스",L"Human",2,4})));
		SpawnableTables.emplace(std::make_pair(TableRowType::ATHLETE_TABLE, &AthleteTable));
		ManagerTable.emplace(std::make_pair(2001, xnew<SpawnableTableRow>(SpawnableTableRow{2001,L"정장-브라운"})));
		ManagerTable.emplace(std::make_pair(2002, xnew<SpawnableTableRow>(SpawnableTableRow{2002,L"정장-블랙"})));
		SpawnableTables.emplace(std::make_pair(TableRowType::MANAGER_TABLE, &ManagerTable));
	}

	~DataTableManager()
	{
		for (auto& tableRow : UniformCostumeTable)
		{
			xdelete(tableRow.second);
		}
		for (auto& tableRow : AthleteTable)
		{
			xdelete(tableRow.second);
		}
		for (auto& tableRow : ManagerTable)
		{
			xdelete(tableRow.second);
		}
	}

public:
	xUnorderedMap<TableRowType, xUnorderedMap<int32, CostumeTableRow*>*> CostumeTables;
	xUnorderedMap<TableRowType, xUnorderedMap<int32, SpawnableTableRow*>*> SpawnableTables;

private:
	xUnorderedMap<int32, CostumeTableRow*> UniformCostumeTable;
	xUnorderedMap<int32, SpawnableTableRow*> AthleteTable;
	xUnorderedMap<int32, SpawnableTableRow*> ManagerTable;
};

extern DataTableManager*				GDataTableManager;