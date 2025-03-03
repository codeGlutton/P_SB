#include "GoogleWidget.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "UMG"

void UGoogleWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);

    _Box.Reset();
}

void UGoogleWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (!_Box.IsValid())
    {
        return;
    }
}

void UGoogleWidget::SetLoginWidget(const TSharedRef<SWidget>& InWidget)
{
    SetWidget(InWidget);

    OnDisplayLogin.Broadcast();
}

void UGoogleWidget::SetSignUpWidget(const TSharedRef<SWidget>& InWidget)
{
    SetWidget(InWidget);

    OnDisplaySignUp.Broadcast();
}

void UGoogleWidget::SetLoginDismissedWidget()
{
    OnDismissedLogin.Broadcast();
}

void UGoogleWidget::SetSignUpDismissedWidget()
{
    OnDismissedSignUp.Broadcast();
}

#if WITH_EDITOR
const FText UGoogleWidget::GetPaletteCategory()
{
    return LOCTEXT("Google", "Google");
}
#endif

void UGoogleWidget::SetWidget(const TSharedRef<SWidget>& InWidget)
{
    if (_Box.IsValid())
    {
        _Box->SetContent(InWidget);
        _Box->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
        // InvalidateLayoutAndVolatility();
    }
}

TSharedRef<SWidget> UGoogleWidget::RebuildWidget()
{
    return SAssignNew(_Box, SBox)
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("웹 로딩중...")))
        ];
}
