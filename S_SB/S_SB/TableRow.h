#pragma once

/************************
	AthleteTableRow
************************/

struct AthleteTableRow
{
	int32 Id;
	xWString ServerName;
	xWString Race;
	int32 Positioin;
	uint8 Skin;
};

/************************
		DataTable
************************/

class DataTable
{
public:
	DataTable()
	{
		AthleteTable.insert(std::make_pair(L"Human", AthleteTableRow{1001,L"Human",L"Human",0,0}));
		AthleteTable.insert(std::make_pair(L"Slime", AthleteTableRow{1002,L"Slime",L"Slime",0,0}));
		AthleteTable.insert(std::make_pair(L"Gearon", AthleteTableRow{1003,L"Gearon",L"Gearon",0,0}));
		AthleteTable.insert(std::make_pair(L"Roo", AthleteTableRow{1004,L"Roo",L"Roo",0,0}));
		AthleteTable.insert(std::make_pair(L"라이타르 캉게스", AthleteTableRow{1005,L"라이타르 캉게스",L"Roo",1,4}));
		AthleteTable.insert(std::make_pair(L"벨 캉가", AthleteTableRow{1006,L"벨 캉가",L"Roo",1,4}));
		AthleteTable.insert(std::make_pair(L"자르 포샤라", AthleteTableRow{1007,L"자르 포샤라",L"Roo",1,3}));
		AthleteTable.insert(std::make_pair(L"쿠엔타르 캉가", AthleteTableRow{1008,L"쿠엔타르 캉가",L"Roo",3,4}));
		AthleteTable.insert(std::make_pair(L"린타 레프리온", AthleteTableRow{1009,L"린타 레프리온",L"Roo",3,1}));
		AthleteTable.insert(std::make_pair(L"벨 스키판", AthleteTableRow{1010,L"벨 스키판",L"Roo",2,2}));
		AthleteTable.insert(std::make_pair(L"크리날 루트하", AthleteTableRow{1011,L"크리날 루트하",L"Roo",2,1}));
		AthleteTable.insert(std::make_pair(L"드랙스 호타르", AthleteTableRow{1012,L"드랙스 호타르",L"Roo",1,3}));
		AthleteTable.insert(std::make_pair(L"프리델 팍스", AthleteTableRow{1013,L"프리델 팍스",L"Roo",2,4}));
		AthleteTable.insert(std::make_pair(L"단테 윌리엄스", AthleteTableRow{1014,L"단테 윌리엄스",L"Human",1,2}));
		AthleteTable.insert(std::make_pair(L"이태성", AthleteTableRow{1015,L"이태성",L"Human",1,1}));
		AthleteTable.insert(std::make_pair(L"윤승재", AthleteTableRow{1016,L"윤승재",L"Human",2,1}));
		AthleteTable.insert(std::make_pair(L"나카모토 하루야", AthleteTableRow{1017,L"나카모토 하루야",L"Human",2,1}));
		AthleteTable.insert(std::make_pair(L"장 웨이", AthleteTableRow{1018,L"장 웨이",L"Human",3,1}));
		AthleteTable.insert(std::make_pair(L"자말 로빈스", AthleteTableRow{1019,L"자말 로빈스",L"Human",2,3}));
		AthleteTable.insert(std::make_pair(L"티아고 산토스", AthleteTableRow{1020,L"티아고 산토스",L"Human",2,3}));
		AthleteTable.insert(std::make_pair(L"미하일 이바노프", AthleteTableRow{1021,L"미하일 이바노프",L"Human",2,2}));
		AthleteTable.insert(std::make_pair(L"이반 코박", AthleteTableRow{1022,L"이반 코박",L"Human",3,2}));
		AthleteTable.insert(std::make_pair(L"잭 카터", AthleteTableRow{1023,L"잭 카터",L"Human",1,3}));
		AthleteTable.insert(std::make_pair(L"이덴 쿠퍼", AthleteTableRow{1024,L"이덴 쿠퍼",L"Human",1,2}));
		AthleteTable.insert(std::make_pair(L"노아 리드", AthleteTableRow{1025,L"노아 리드",L"Human",1,2}));
		AthleteTable.insert(std::make_pair(L"아니크 로이", AthleteTableRow{1026,L"아니크 로이",L"Human",2,4}));
		AthleteTable.insert(std::make_pair(L"마테오 라미레스", AthleteTableRow{1027,L"마테오 라미레스",L"Human",2,4}));
	}

public:
	xUnorderedMap <xWString, AthleteTableRow> AthleteTable;
};

extern DataTable*				GDataTable;