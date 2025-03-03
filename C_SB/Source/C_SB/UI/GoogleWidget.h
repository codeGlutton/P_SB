#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "GoogleWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisplayGooglePopup);

/**
 * 구글 위젯을 띄우기 위한 위젯
 */
UCLASS(meta = (DisplayName = "Google Popup"))
class C_SB_API UGoogleWidget : public UWidget
{
	GENERATED_BODY()
	
public:
	virtual void					ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void					SynchronizeProperties() override;

public:
	void							SetLoginWidget(const TSharedRef<SWidget>& InWidget);
	void							SetSignUpWidget(const TSharedRef<SWidget>& InWidget);
	void							SetLoginDismissedWidget();
	void							SetSignUpDismissedWidget();

#if WITH_EDITOR
	virtual const FText				GetPaletteCategory() override;
#endif

protected:
	void							SetWidget(const TSharedRef<SWidget>& InWidget);

    virtual TSharedRef<SWidget>		RebuildWidget() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Google Popup|Event")
	FOnDisplayGooglePopup			OnDisplayLogin;

	UPROPERTY(BlueprintAssignable, Category = "Google Popup|Event")
	FOnDisplayGooglePopup			OnDisplaySignUp;

	UPROPERTY(BlueprintAssignable, Category = "Google Popup|Event")
	FOnDisplayGooglePopup			OnDismissedLogin;

	UPROPERTY(BlueprintAssignable, Category = "Google Popup|Event")
	FOnDisplayGooglePopup			OnDismissedSignUp;

private:
	TSharedPtr<class SBox>			_Box;
};
