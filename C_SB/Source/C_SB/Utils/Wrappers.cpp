#include "Utils/Wrappers.h"
#include "ByteConverters.h"
#include "C_SB.h"

/************************
   FRegexPatternWrapper
*************************/

FRegexPatternWrapper& FRegexPatternWrapper::operator=(const FRegexPattern& Other)
{
	Pattern = Other;

	return *this;
}

FRegexPatternWrapper& FRegexPatternWrapper::operator=(FRegexPattern&& Other)
{
	Pattern = MoveTemp(Other);
	
	return *this;
}

/*************************
	FSBServerSelectInfo
*************************/

FSBServerSelectInfo::FSBServerSelectInfo(const Protocol::ServerSelectInfo& ServerSeclectInfo) : ServerId(ServerSeclectInfo.server_id()), Dencity(ServerSeclectInfo.density())
{
	Utils::UTF8To16(ServerSeclectInfo.name(), Name);
}

FSBServerSelectInfo::FSBServerSelectInfo(Protocol::ServerSelectInfo&& ServerSeclectInfo) : ServerId(ServerSeclectInfo.server_id()), Dencity(ServerSeclectInfo.density())
{
	Utils::UTF8To16(ServerSeclectInfo.name(), Name);
}

FSBServerSelectInfo::FSBServerSelectInfo(const Protocol::ServerInfo& ServerInfo) : ServerId(ServerInfo.server_id()), Dencity(ServerInfo.density())
{
	Utils::UTF8To16(ServerInfo.name(), Name);
}

FSBServerSelectInfo::FSBServerSelectInfo(Protocol::ServerInfo&& ServerInfo) : ServerId(ServerInfo.server_id()), Dencity(ServerInfo.density())
{
	Utils::UTF8To16(ServerInfo.name(), Name);
}

bool FSBServerSelectInfo::operator==(FSBServerSelectInfo const& Other) const
{
	return ServerId == Other.ServerId && Name == Other.Name && Dencity == Other.Dencity;
}

/*************************
	FSBPlayerSelectInfo
*************************/

FSBPlayerSelectInfo::FSBPlayerSelectInfo(const int32& PlayerDbId, const int32& PlayerTableId, const FString& Name, const int32& CostumeSetting, const int32& Score, const google::protobuf::Timestamp& CreatedTime) : PlayerDbId(PlayerDbId), PlayerTableId(PlayerTableId), Name(Name), Score(Score)
{
	CostumeSettingConvertor Converter(CostumeSetting);
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;

	this->CreatedTime = FDateTime::FromUnixTimestamp(CreatedTime.seconds());
}

FSBPlayerSelectInfo::FSBPlayerSelectInfo(int32&& PlayerDbId, int32& PlayerTableId, FString&& Name, int32&& CostumeSetting, int32&& Score, google::protobuf::Timestamp&& CreatedTime) : PlayerDbId(MoveTemp(PlayerDbId)), PlayerTableId(MoveTemp(PlayerTableId)), Name(MoveTemp(Name)), Score(MoveTemp(Score))
{
	CostumeSettingConvertor Converter(MoveTemp(CostumeSetting));
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;

	this->CreatedTime = FDateTime::FromUnixTimestamp(CreatedTime.seconds());
}

FSBPlayerSelectInfo::FSBPlayerSelectInfo(const Protocol::PlayerSelectInfo& PlayerSeclectInfo) : PlayerDbId(PlayerSeclectInfo.player_db_id()), PlayerTableId(PlayerSeclectInfo.player_table_id()), Score(PlayerSeclectInfo.rank_score())
{
	Utils::UTF8To16(PlayerSeclectInfo.name(), Name);

	CostumeSettingConvertor Converter(PlayerSeclectInfo.costume_setting());
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;

	CreatedTime = FDateTime::FromUnixTimestamp(PlayerSeclectInfo.created_time().seconds());
}

FSBPlayerSelectInfo::FSBPlayerSelectInfo(Protocol::PlayerSelectInfo&& PlayerSeclectInfo) : PlayerDbId(PlayerSeclectInfo.player_db_id()), PlayerTableId(PlayerSeclectInfo.player_table_id()), Score(PlayerSeclectInfo.rank_score())
{
	Utils::UTF8To16(PlayerSeclectInfo.name(), Name);

	CostumeSettingConvertor Converter(PlayerSeclectInfo.costume_setting());
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;

	CreatedTime = FDateTime::FromUnixTimestamp(PlayerSeclectInfo.created_time().seconds());
}

bool FSBPlayerSelectInfo::operator==(FSBPlayerSelectInfo const& Other) const
{
	return PlayerDbId == Other.PlayerDbId && PlayerTableId == Other.PlayerTableId && Name == Other.Name && TeamUniform == Other.TeamUniform && TeamShoes == Other.TeamShoes && TeamAccessories == Other.TeamAccessories && Score == Other.Score && CreatedTime == Other.CreatedTime;
}

/************************
		Functions
*************************/

ESBTier USBFunctionLibrary::GetTier(const int32& Score)
{
	uint8 Tier = FMath::Clamp(StaticCast<uint8>(RankingConvertor::GetTier(Score)), StaticCast<uint8>(ESBTier::BRONZE), StaticCast<uint8>(ESBTier::DIAMOND));
	return StaticCast<ESBTier>(Tier);
}
