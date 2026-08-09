// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameOverWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/PlayerProfileComponent/C_PlayerProfileComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void UC_GameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UC_GameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ExitToLobbyButton 바인딩
	if (ExitToLobbyButton)
	{
		ExitToLobbyButton->OnClicked.AddDynamic(this, &UC_GameOverWidget::OnExitToLobbyButtonClicked);
	}
}

void UC_GameOverWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	HandleExitToLobbyTimer(InDeltaTime);
}

void UC_GameOverWidget::ActivateWinningSequence()
{
	InitLocalPlayerRanking();
	
	this->SetVisibility(ESlateVisibility::HitTestInvisible);
	
	AC_BasicPlayer* MyPlayer = LEVEL_MANAGER->GetLocalPlayer();
	
	PlayerCharacterName->SetText(FText::FromString(MyPlayer->GetPlayerProfileComponent()->GetPlayerName()));
	
	FString TotalCharacterString = " / " + FString::FromInt(LEVEL_MANAGER->GetPlayers().Num());
	
	RankingTextTotalCharacterCount->SetText(FText::FromString(TotalCharacterString));
	const FString RankingStr = "#" + FString::FromInt(m_Ranking);
	RankingText->SetText(FText::FromString(RankingStr));

	KillCountText->SetText(FText::FromString(FString::FromInt(MyPlayer->GetKillCount())));

	PlayAnimation(WinningChickenAnimation);
}

void UC_GameOverWidget::ActivateLoseSequence()
{
	InitLocalPlayerRanking();
	
	this->SetVisibility(ESlateVisibility::Visible);

	AC_BasicPlayer* MyPlayer                   = LEVEL_MANAGER->GetLocalPlayer();
	AC_BasicPlayerController* PlayerController = MyPlayer->GetController<AC_BasicPlayerController>();

	PlayerController->SetInputMode
	(
		FInputModeGameAndUI().
		SetWidgetToFocus(this->TakeWidget()).
		SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock).
		SetHideCursorDuringCapture(false)
	);

	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetIgnoreLookInput(true);

	PlayerCharacterName->SetText(FText::FromString(MyPlayer->GetPlayerProfileComponent()->GetPlayerName()));
	RankingTextTopRight->SetText(FText::FromString(FString::FromInt(m_Ranking)));
	
	FString TotalCharacterString = " / " + FString::FromInt(LEVEL_MANAGER->GetPlayers().Num());
	RankingTextTotalCharacterCount->SetText(FText::FromString(TotalCharacterString));

	FString RankingString = "#" + FString::FromInt(1);
	RankingText->SetText(FText::FromString(RankingString));

	KillCountText->SetText(FText::FromString(FString::FromInt(MyPlayer->GetKillCount())));

	PlayAnimation(LoseAnimation);
}

void UC_GameOverWidget::InitLocalPlayerRanking()
{
	const int32 MyKillCount = LEVEL_MANAGER->GetLocalPlayer()->GetKillCount();
	m_Ranking = 1;
	
	for (AC_BasicPlayer* Player : LEVEL_MANAGER->GetPlayers())
	{
		// 나보다 KillCount가 높은 플레이어 수만큼 등수 밀어냄
		if (Player->GetKillCount() > MyKillCount)
			++m_Ranking;
	}
}

void UC_GameOverWidget::HandleExitToLobbyTimer(float DeltaTime)
{
	if (ExitToLobbyCountDownText->GetRenderOpacity() < 0.95f) return;

	ExitToLobbyTimer -= DeltaTime;

	int Sec = static_cast<int>(ExitToLobbyTimer) + 1;
	FString CountDownString = "You will exit to lobby in " + FString::FromInt(Sec) + " seconds";
	ExitToLobbyCountDownText->SetText(FText::FromString(CountDownString));

	// 60초 카운트다운 완료 시 로비 이동
	if (ExitToLobbyTimer <= 0.f)
	{
		ExitToLobbyTimer = 999.f; // 중복 호출 방지
		OnExitToLobbyButtonClicked();
	}
}

void UC_GameOverWidget::OnExitToLobbyButtonClicked()
{
	if (bIsExiting) return;
	bIsExiting = true; // 중복 실행 방지

	// 스팀/온라인 세션 정리 후 스타트 레벨로 이동
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateLambda(
				[this](FName SessionName, bool bWasSuccessful)
				{
					UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), LobbyLevel);
				}
			));
			return;
		}
	}

	// 2. 세션이 없을 경우 즉시 레벨 이동
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), LobbyLevel);
}