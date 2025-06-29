#include "SBBaseWidgetViewModel.h"

/************************
 USBCheckSwitcherViewModel
*************************/

void USBCheckSwitcherViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();
}

/************************
	USBNoticeViewModel
*************************/

void USBNoticeViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();
}

void USBNoticeViewModel::ClearNotice()
{
	SetNoticeTitle(FText::GetEmpty());
	SetNoticeMsg(FText::GetEmpty());
}

void USBPostTextViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();
}
