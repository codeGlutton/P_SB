#pragma once
#include "Struct.pb.h"
#include "Types.h"

// 컨버텅의 반복문 기능 적용 매크로
#define USE_ARRAY_CONVERTOR(returnType, allBytes, elementByte)												\
	returnType& operator[](int32 index)																		\
	{																										\
		return *reinterpret_cast<returnType*>(reinterpret_cast<BYTE*>(&allBytes) + index * elementByte);	\
	}																										\
																											\
	returnType* begin()																						\
	{																										\
		return reinterpret_cast<returnType*>(&allBytes);													\
	}																										\
																											\
	returnType* end()																						\
	{																										\
		return reinterpret_cast<returnType*>(&allBytes + 1);												\
	}

struct IdConvertor
{
	IdConvertor(const uint64& objId) : objId(objId)
	{
	}
	IdConvertor(uint64&& objId) : objId(objId)
	{
	}
	IdConvertor(uint32 gameId, uint16 tableId)
	{
		objId = ((uint64)tableId << 48) + (uint64)gameId;
	}
	IdConvertor(uint32 gameId, uint32 dbId, uint16 tableId)
	{
		objId = ((uint64)tableId << 48) + ((uint64)dbId << 24) + (uint64)gameId;
	}

	union
	{
		struct
		{
			uint64					gameId : 24;
			uint64					dbId : 24;
			uint64					tableId : 16;
		};
		uint64						objId;
	};
};

/* 유저 조작 변수 컨버터 */

struct TacticsConvertor
{
	TacticsConvertor(const uint32& inBytes) : bytes(inBytes)
	{
	}
	TacticsConvertor(uint32&& inBytes) : bytes(inBytes)
	{
	}

	USE_ARRAY_CONVERTOR(Protocol::TacticsType, bytes, 8)

		union
	{
		struct
		{
			Protocol::TacticsType	tactics0 : 8;
			Protocol::TacticsType	tactics1 : 8;
			Protocol::TacticsType	tactics2 : 8;
			Protocol::TacticsType	tactics3 : 8;
		};
		uint32						bytes;
	};
};

struct BreakCountsConvertor
{
	BreakCountsConvertor(const uint32& inBytes) : bytes(inBytes)
	{
	}
	BreakCountsConvertor(uint32&& inBytes) : bytes(inBytes)
	{
	}

	union
	{
		struct
		{
			uint16					timeOutCount;
			uint16					substitutionCount;
		};
		uint32						bytes;
	};
};

struct CostumeSettingConvertor
{
	CostumeSettingConvertor(const uint32& inBytes) : bytes(inBytes)
	{
	}
	CostumeSettingConvertor(uint32&& inBytes) : bytes(inBytes)
	{
	}
	CostumeSettingConvertor(uint32 uniform, uint32 shoes, uint32 accessories)
	{
		bytes = (accessories << 20) + (shoes << 10) + uniform;
	}

	union
	{
		struct
		{
			uint32					uniform : 10;
			uint32					shoes : 10;
			uint32					accessories : 12;
		};
		uint32						bytes;
	};
};

/* 결과 데이터 컨버터들 */

struct PlayerComputedInfoConvertor
{
	// TODO
};

struct ScoreBoardConvertor
{
	ScoreBoardConvertor(const uint64& inBytes) : bytes(inBytes)
	{
	}
	ScoreBoardConvertor(uint64&& inBytes) : bytes(inBytes)
	{
	}

	union
	{
		struct
		{
			uint64					min : 5;
			uint64					to : 4;
			uint64					pf : 4;
			uint64					blk : 4;
			uint64					stl : 4;
			uint64					drb : 5;
			uint64					orb : 4;
			uint64					ast : 5;
			uint64					trial3pt : 5;
			uint64					score3pt : 4;
			uint64					trial2pt : 5;
			uint64					score2pt : 5;
			uint64					trialFt : 5;
			uint64					scoreFt : 5;
		};
		uint64						bytes;
	};
};

/* 선수 능력치 데이터 컨버터들 */

struct PhysicalInfoConvertor
{
	PhysicalInfoConvertor(const uint64& inBytes) : bytes(inBytes)
	{
	}
	PhysicalInfoConvertor(uint64&& inBytes) : bytes(inBytes)
	{
	}

	union
	{
		struct
		{
			uint64					sargentJump : 8;
			uint64					firstStep : 8;
			uint64					fullSpeed : 8;
			uint64					sideStep : 8;
			uint64					power : 8;
			uint64					height : 12;
			uint64					wingSpan : 12;
		};
		uint64						bytes;
	};
};

struct MasteryConvertor
{
	MasteryConvertor(const uint64& inBytes) : bytes(inBytes)
	{
	}
	MasteryConvertor(uint64&& inBytes) : bytes(inBytes)
	{
	}

	union
	{
		struct
		{
			// 볼키핑, 스틸 능력
			uint8					handling;
			// 공격 및 수비 전술 이해도
			uint8					bq;
			// 리바운드, 블락 능력
			uint8					reactionTiming;
			// 자유투, 슛 능력
			uint8					shooting;
			// 스크린, 컷인 능력
			uint8					spacePerception;
			// 레이업, 플로터 능력
			uint8					ballTouching;
			// 어시스트, 경로차단 능력
			uint8					passing;
			// 파울 겟, 회피, 플랍 능력
			uint8					foulManaging;
		};
		uint64						bytes;
	};
};

struct InjuryInfoConvertor
{
	InjuryInfoConvertor(const uint32& inBytes) : bytes(inBytes)
	{
	}
	InjuryInfoConvertor(uint32&& inBytes) : bytes(inBytes)
	{
	}

	union
	{
		struct
		{
			Protocol::Bodypart		bodypart : 16;
			Protocol::InjuryType	type : 16;
		};
		uint32						bytes;
	};
};

struct TagsConvertor
{
	TagsConvertor(const uint32& inBytes) : bytes(inBytes)
	{
	}
	TagsConvertor(uint32&& inBytes) : bytes(inBytes)
	{
	}

	USE_ARRAY_CONVERTOR(Protocol::AthleteTag, bytes, 8)

		union
	{
		struct
		{
			Protocol::AthleteTag	tag0 : 8;
			Protocol::AthleteTag	tag1 : 8;
			Protocol::AthleteTag	tag2 : 8;
			Protocol::AthleteTag	tag3 : 8;
		};
		uint32						bytes;
	};
};

/* 컨버팅 함수 */

struct RankingConvertor
{
	enum Tier : uint8
	{
		TIER_BRONZE = 0,
		TIER_SILVER,
		TIER_GOLD,
		TIER_PLATINUM,
		TIER_DIAMOND
	};

	static Tier GetTier(const int32& score)
	{
		return (Tier)(std::clamp((uint8)(score / 100), (uint8)TIER_BRONZE, (uint8)TIER_DIAMOND));
	}
};
