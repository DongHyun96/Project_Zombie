


#include "C_MenuWidget.h"
#include "Input/Reply.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void UC_MenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	bIsFocusable = true;
}

void UC_MenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Socials)
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			if (Pawn->HasAuthority())
			{
				Button_Socials->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				Button_Socials->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		Button_Socials->OnClicked.AddDynamic(this, &UC_MenuWidget::OnSocialsButtonClicked);
	}

	// Button_Exit_Game 바인딩 확인
	if (Button_Exit_Game)
	{
		Button_Exit_Game->OnClicked.AddDynamic(this, &UC_MenuWidget::OnExitGameButtonClicked);
	}

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

void UC_MenuWidget::OnExitGameButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();

	// 1. 스팀/온라인 세션 파괴 후 안전 종료
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 스팀 세션을 먼저 종료(Destroy)하고, 완료 콜백에서 QuitGame 수행
			SessionInterface->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateLambda(
				[this, PC](FName SessionName, bool bWasSuccessful)
				{
					UKismetSystemLibrary::QuitGame(
						GetWorld(),
						PC,
						EQuitPreference::Quit,
						true
					);
				}
			));
			return;
		}
	}

	// 2. 세션 인터페이스가 없거나 싱글 환경일 경우 즉시 종료
	UKismetSystemLibrary::QuitGame(
		GetWorld(),
		PC,
		EQuitPreference::Quit,
		true
	);
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