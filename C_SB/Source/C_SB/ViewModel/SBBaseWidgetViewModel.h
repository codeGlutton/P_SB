#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Components/SlateWrapperTypes.h"
#include "SBBaseWidgetViewModel.generated.h"

#define UE_MVVM_ADD_NOTIFY(NotifyName)													\
	FDelegateHandle NotifyName##NotifyHandle;											\
	void Notify_##NotifyName##(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)

#define UE_MVVM_BIND_ON_VALUE_CHANGED(Class, Object, MemberName, NotifyName)			\
	NotifyName##NotifyHandle = Object->AddFieldValueChangedDelegate(Class::FFieldNotificationClassDescriptor::MemberName, INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &ThisClass::Notify_##NotifyName))

#define UE_MVVM_UNBIND_ON_VALUE_CHANGED(Class, Object, MemberName, NotifyName)			\
	Object->RemoveFieldValueChangedDelegate(Class::FFieldNotificationClassDescriptor::MemberName, NotifyName##NotifyHandle)


UCLASS(Abstract)
class C_SB_API USBViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewModel")
	void Initialize();
	virtual void Initialize_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ViewModel")
	void Deinitialize();
	virtual void Deinitialize_Implementation() {}
};

/************************
 USBCheckSwitcherViewModel
*************************/

namespace ECheckSwitcherState
{
	enum Type
	{
		Fail = 0,
		Success = 1
	};
}

/**
 * 
 */
UCLASS()
class C_SB_API USBCheckSwitcherViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBCheckSwitcherViewModel() : SwitcherIndex(0)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetSwitcherIndex(int32 NewSwitcherIndex)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(SwitcherIndex, NewSwitcherIndex))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSuccess);
		}
	}

	int32 GetSwitcherIndex() const
	{
		return SwitcherIndex;
	}

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsSuccess() const
	{
		return SwitcherIndex == ECheckSwitcherState::Success;
	}

public:
	/* 수동 실행 함수 */

	virtual void Initialize_Implementation() override;

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32											SwitcherIndex;
};

/************************
	USBNoticeViewModel
*************************/

UCLASS(BlueprintType)
class C_SB_API USBNoticeViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBNoticeViewModel() : NoticeTitle(FText::GetEmpty()), NoticeMsg(FText::GetEmpty())
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetNoticeTitle(const FText& NewNoticeTitle)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(NoticeTitle, NewNoticeTitle))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNoticeVisibility);
		}
	}

	void SetNoticeMsg(const FText& NewNoticeMsg)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(NoticeMsg, NewNoticeMsg))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNoticeVisibility);
		}
	}

	const FText& GetNoticeTitle() const
	{
		return NoticeTitle;
	}

	const FText& GetNoticeMsg() const
	{
		return NoticeMsg;
	}

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetNoticeVisibility() const
	{
		return (NoticeMsg.IsEmpty() && NoticeTitle.IsEmpty()) ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	}

public:
	/* 수동 실행 함수 */

	virtual void Initialize_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void											ClearNotice();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											NoticeTitle;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											NoticeMsg;
};

/************************
   USBPostTextViewModel
*************************/

UCLASS(BlueprintType)
class C_SB_API USBPostTextViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBPostTextViewModel() : TargetText(FText::GetEmpty()), bIsSuccessed(false)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetTargetText(const FText& NewText)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(TargetText, NewText))
		{
		}
	}

	void SetIsSuccessed(bool bNewIsSuccessed)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bIsSuccessed, bNewIsSuccessed))
		{
		}
	}

	const FText& GetTargetText() const
	{
		return TargetText;
	}

	bool GetIsSuccessed() const
	{
		return bIsSuccessed;
	}

public:
	/* FieldNotify Function */

public:
	/* 수동 실행 함수 */

	virtual void Initialize_Implementation() override;

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											TargetText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetIsSuccessed", Getter = "GetIsSuccessed", meta = (AllowPrivateAccess))
	bool											bIsSuccessed;
};
