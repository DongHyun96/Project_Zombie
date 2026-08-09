



#include "C_MenuWidget.h"
#include "Input/Reply.h"
#include "Components/Button.h"
#include "Actor/Character/Player/C_BasicPlayer.h"

// 생성자 코드 삭제 후 NativeOnInitialized 구현
void UC_MenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯 포커스 가능 설정
	bIsFocusable = true;
}

void UC_MenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Socials)
	{
		Button_Socials->OnClicked.AddDynamic(this, &UC_MenuWidget::OnSocialsButtonClicked);
	}

	// 켜질 때 포커스 강제 세팅
	SetKeyboardFocus();
}

void UC_MenuWidget::OnSocialsButtonClicked()
{
	if (!WBP_FriendList) return;

	if (WBP_FriendList->GetVisibility() == ESlateVisibility::Visible)
	{
		WBP_FriendList->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		WBP_FriendList->SetVisibility(ESlateVisibility::Visible);
	}
}

FReply UC_MenuWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(Pawn))
			{
				Player->ToggleMenuWidget();
				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}