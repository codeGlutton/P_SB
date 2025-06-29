#include "SBPlayerSetupViewModel.h"
#include "Engine/AssetManager.h"
#include "SBGameModeBase.h"
#include "Utils.h"
#include "C_SB.h"

/************************
 USBPlayerSelectViewModel
*************************/

void USBPlayerSelectViewModel::InitViewModel(const TScriptInterface<ISBCustomizing>& NewManagerCharacter)
{
	SetManagerCharacter(NewManagerCharacter);
}

void USBPlayerSelectViewModel::UpdateManagerOutfit()
{
	ASBGameModeBase* SBGameModeBase = Utils::GetWorld()->GetAuthGameMode<ASBGameModeBase>();
	if (SBGameModeBase == nullptr)
		return;
	TObjectPtr<UDataTable>* ManagerDataTable = SBGameModeBase->SpawnableTables.Find(ESBTableRowType::MANAGER_TABLE);
	if (ManagerDataTable == nullptr)
		return;

	if (PlayerSelectInfo.Name.IsEmpty() == true)
		return;

	auto* TableRow = (*ManagerDataTable)->FindRow<FSBSpawnableTableRow>(FName(FString::FromInt(PlayerSelectInfo.PlayerTableId)), FString());
	if (TableRow == nullptr)
		return;

	TArray<FSoftObjectPath> LoadPaths{ TableRow->ClientMesh.ToSoftObjectPath(), TableRow->ClientMaterial.ToSoftObjectPath() };
	ManagerCharacter->ChangeOutfit(ESBTableRowType::MANAGER_TABLE, TableRow->ClientMesh, TableRow->ClientMaterial, true);
}

/************************
 USBPlayerCreateViewModel
*************************/

void USBPlayerCreateViewModel::InitViewModel(const TScriptInterface<ISBCustomizing>& NewManagerCharacter, const TScriptInterface<ISBCustomizing>& NewAthleteCharacter, bool bUseCostumizing)
{
	SetManagerCharacter(NewManagerCharacter);
	SetAthleteCharacter(NewAthleteCharacter);

	if (bUseCostumizing == true)
	{
		LoadTableRows();
	}
}

void USBPlayerCreateViewModel::CreateNewPlayer(const int32& EmptyPlayerDBId)
{
	PlayerDBId = EmptyPlayerDBId;
}

void USBPlayerCreateViewModel::GetNewPlayerInfo(FSBPlayerSelectInfo& NewPlayerInfo)
{
	NewPlayerInfo.PlayerDbId = PlayerDBId;
	NewPlayerInfo.Name = Name.ToString();
	NewPlayerInfo.PlayerTableId = ManagerTableId;
	NewPlayerInfo.TeamUniform = UniformTableId;
	// TODO
}

void USBPlayerCreateViewModel::ChangeManagerTableId(const int32& Offset)
{
	if (ManagerTableRows.IsEmpty() == true)
		return;

	ManagerTableIndex = (Offset + ManagerTableIndex + ManagerTableRows.Num()) % ManagerTableRows.Num();
	SetManagerTableId(ManagerTableRows[ManagerTableIndex]->Id);
	SetName(FText::FromString(ManagerTableRows[ManagerTableIndex]->ClientName));
}

void USBPlayerCreateViewModel::ChangeUniformTableId(const int32& Offset)
{
	if (ManagerTableRows.IsEmpty() == true)
		return;

	UniformTableIndex = (Offset + UniformTableIndex + UniformTableRows.Num()) % UniformTableRows.Num();
	SetUniformTableId(UniformTableRows[UniformTableId]->Id);
	SetName(FText::FromString(UniformTableRows[UniformTableId]->ClientName));
}

void USBPlayerCreateViewModel::UpdateManagerOutfit()
{
	ASBGameModeBase* SBGameModeBase = Utils::GetWorld()->GetAuthGameMode<ASBGameModeBase>();
	if (SBGameModeBase == nullptr)
		return;
	TObjectPtr<UDataTable>* ManagerDataTable = SBGameModeBase->SpawnableTables.Find(ESBTableRowType::MANAGER_TABLE);
	if (ManagerDataTable == nullptr)
		return;

	auto* TableRow = (*ManagerDataTable)->FindRow<FSBSpawnableTableRow>(FName(FString::FromInt(ManagerTableId)), FString());
	if (TableRow == nullptr)
		return;

	TArray<FSoftObjectPath> LoadPaths{ TableRow->ClientMesh.ToSoftObjectPath(), TableRow->ClientMaterial.ToSoftObjectPath() };
	ManagerCharacter->ChangeOutfit(ESBTableRowType::MANAGER_TABLE, TableRow->ClientMesh, TableRow->ClientMaterial, true);
}

void USBPlayerCreateViewModel::UpdateAthleteOutfit(const ESBTableRowType& CostumeType)
{
	ASBGameModeBase* SBGameModeBase = Utils::GetWorld()->GetAuthGameMode<ASBGameModeBase>();
	if (SBGameModeBase == nullptr)
		return;
	TObjectPtr<UDataTable>* AthleteCostumeDataTable = SBGameModeBase->CostumeTables.Find(CostumeType);
	if (AthleteCostumeDataTable == nullptr)
		return;

	FSBCostumeTableRow* TableRow = nullptr;
	switch (CostumeType)
	{
	case ESBTableRowType::UNIFORM_COSTUME_TABLE:
		TableRow = (*AthleteCostumeDataTable)->FindRow<FSBCostumeTableRow>(FName(FString::FromInt(UniformTableId)), FString());
		break;
	default:
		break;
	}

	if (TableRow == nullptr)
		return;
	TArray<FSoftObjectPath> LoadPaths{ TableRow->ClientMesh.ToSoftObjectPath(), TableRow->ClientMaterial.ToSoftObjectPath() };
	AthleteCharacter->ChangeOutfit(CostumeType, TableRow->ClientMesh, TableRow->ClientMaterial, true);
}

void USBPlayerCreateViewModel::LoadTableRows()
{
	ASBGameModeBase* SBGameModeBase = Utils::GetWorld()->GetAuthGameMode<ASBGameModeBase>();
	if (SBGameModeBase == nullptr)
		return;

	TObjectPtr<UDataTable>* ManagerDataTable = SBGameModeBase->SpawnableTables.Find(ESBTableRowType::MANAGER_TABLE);
	if (ManagerDataTable != nullptr)
	{
		(*ManagerDataTable)->GetAllRows<FSBTableRow>(FString(), ManagerTableRows);
	}

	TObjectPtr<UDataTable>* UniformDataTable = SBGameModeBase->CostumeTables.Find(ESBTableRowType::UNIFORM_COSTUME_TABLE);
	if (UniformDataTable != nullptr)
	{
		(*UniformDataTable)->GetAllRows<FSBTableRow>(FString(), UniformTableRows);
	}
	// TODO
}

/************************
 USBPlayerSetupViewModel
*************************/

USBPlayerSetupViewModel::USBPlayerSetupViewModel()
{
}

void USBPlayerSetupViewModel::InitViewModel(TArray<TScriptInterface<ISBCustomizing>> SelectManagers, TScriptInterface<ISBCustomizing> CreateManager, TScriptInterface<ISBCustomizing> CreateAthlete, bool bUseCustomizing)
{
	if (SelectManagers.Num() != 3)
		return;

	SetPlayerSelectViewModel1(NewObject<USBPlayerSelectViewModel>());
	PlayerSelectViewModel1->InitViewModel(SelectManagers[0]);
	SetPlayerSelectViewModel2(NewObject<USBPlayerSelectViewModel>());
	PlayerSelectViewModel2->InitViewModel(SelectManagers[1]);
	SetPlayerSelectViewModel3(NewObject<USBPlayerSelectViewModel>());
	PlayerSelectViewModel3->InitViewModel(SelectManagers[2]);
	
	SetPlayerCreateViewModel(NewObject<USBPlayerCreateViewModel>());
	PlayerCreateViewModel->InitViewModel(CreateManager, CreateAthlete, bUseCustomizing);
}
