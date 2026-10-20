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
#include "Actor/GameOverChecker/C_GameOverChecker.h"
#include "GameFramework/PlayerState.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/ChapterManager/C_StageManager.h"
#include "Interfaces/OnlineSessionInterface.h"

void UC_GameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// ExitToLobbyButton 바인딩
	if (ExitToLobbyButton) ExitToLobbyButton->OnClicked.AddDynamic(this, &UC_GameOverWidget::OnExitToLobbyButtonClicked);
}

void UC_GameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_GameOverWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	HandleExitToLobbyTimer(InDeltaTime);
}

void UC_GameOverWidget::ActivateWinningSequence()
{
	InitLocalPlayerRanking();

	this->SetVisibility(ESlateVisibility::Visible);

	AC_BasicPlayer* MyPlayer = LEVEL_MANAGER->GetLocalPlayer();
	if (!MyPlayer)
	{
		PRINT_LOCAL
		(
			GetWorld(),
			"[UC_GameOverWidget::ActivateWinningSequence] : LocalPlayer destroyed! May be currently shutting down the level or game itself.",
			FColor::Red,
			10.f
		);
		return;
	}

	PlayerCharacterName->SetText(FText::FromString(MyPlayer->GetPlayerState()->GetPlayerName()));

	FString TotalCharacterString = " / " + FString::FromInt(LEVEL_MANAGER->GetPlayers().Num());

	RankingTextTotalCharacterCount->SetText(FText::FromString(TotalCharacterString));
	const FString RankingStr = "#" + FString::FromInt(m_Ranking);
	RankingText->SetText(FText::FromString(RankingStr));
	RankingTextTopRight->SetText(FText::FromString(FString::FromInt(m_Ranking)));

	KillCountText->SetText(FText::FromString(FString::FromInt(MyPlayer->GetKillCount())));

	// 서버는 ExitToLobby 버튼 있는 Animation, 클라의 경우 없는 Animation으로 처리
	// 또한 서버의 경우, Stage를 다음 Stage로 여기서 맞춤 처리함
	if (MyPlayer->HasAuthority())
	{
		PlayAnimation(WinningChickenAnimation);
		ExitToLobbyButton->SetVisibility(ESlateVisibility::Visible);
		
		// Stage를 다음 Stage로 여기서 맞춤
		GetGameInstance()->GetSubsystem<UC_StageManager>()->SetCurStageToNextStage();

		APlayerController* PC = MyPlayer->GetController<APlayerController>();
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode{};
		PC->SetInputMode(InputMode);
	}
	else
	{
		PlayAnimation(Client_WinningChickenAnimation);
		ExitToLobbyButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UC_GameOverWidget::ActivateLoseSequence()
{
	InitLocalPlayerRanking();

	AC_BasicPlayer* MyPlayer = LEVEL_MANAGER->GetLocalPlayer();
	if (!MyPlayer)
	{
		PRINT_LOCAL
		(
			GetWorld(),
			"[UC_GameOverWidget::ActivateLoseSequence] : LocalPlayer destroyed! May be currently shutting down the level or game itself.",
			FColor::Red,
			10.f
		);
		return;
	}

	this->SetVisibility(ESlateVisibility::Visible);

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

	PlayerCharacterName->SetText(FText::FromString(MyPlayer->GetPlayerState()->GetPlayerName()));
	RankingTextTopRight->SetText(FText::FromString(FString::FromInt(m_Ranking)));

	FString TotalCharacterString = " / " + FString::FromInt(LEVEL_MANAGER->GetPlayers().Num());
	RankingTextTotalCharacterCount->SetText(FText::FromString(TotalCharacterString));

	FString RankingString = "#" + FString::FromInt(1);
	RankingText->SetText(FText::FromString(RankingString));

	KillCountText->SetText(FText::FromString(FString::FromInt(MyPlayer->GetKillCount())));

	if (MyPlayer->HasAuthority())
	{
		PlayAnimation(LoseAnimation);
		ExitToLobbyButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		PlayAnimation(Client_LoseAnimation);
		ExitToLobbyButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UC_GameOverWidget::UpdateExitToLobbyCountDownRemainSec(int32 _RemainSec)
{
	FString CountDownString = "You will exit to lobby in " + FString::FromInt(_RemainSec) + " seconds";
	ExitToLobbyCountDownText->SetText(FText::FromString(CountDownString));
}

void UC_GameOverWidget::InitLocalPlayerRanking()
{
	AC_BasicPlayer* MyPlayer = LEVEL_MANAGER->GetLocalPlayer();
	if (!MyPlayer)
	{
		PRINT_LOCAL
		(
			GetWorld(),
			"[UC_GameOverWidget::InitLocalPlayerRanking] : LocalPlayer destroyed! May be currently shutting down the level or game itself.",
			FColor::Red,
			10.f
		);
		return;
	}
	const int32 MyKillCount = MyPlayer->GetKillCount();

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
	AC_BasicPlayer* MyPlayer = LEVEL_MANAGER->GetLocalPlayer();
	if (!MyPlayer || !MyPlayer->HasAuthority()) return; // 오로지 서버 쪽에서만 ExitToLobbyTimer 시간 재고, 클라 환경으로 뿌려줌
	
	// 실제로 화면에 ExitToLobbyCountDown Text가 보이는 경우에 시간 재기
	if (ExitToLobbyCountDownText->GetRenderOpacity() < 0.95f) return;

	ExitToLobbyTimer -= DeltaTime;

	const int32 CurSecForUI = static_cast<int32>(ExitToLobbyTimer) + 1;

	// 이전 Tick에서 표기한 Second와 다를 때에만 실질적인 UI 업데이트 및 클라이언트 시간 동기화 처리
	if (CurSecForUI != PrevExitToLobbyTimerForUI)
	{
		AC_GameOverChecker* GOChecker = GAME_LV_GAME_MODE(GetWorld())->GetGameOverChecker();
		
		if (!GOChecker)
		{
			PRINT_LOCAL(GetWorld(), "[UC_GameOverWidget::HandleExitToLobbyTimer] : GameOverChecker is nullptr!", FColor::Red, 10.f);
			return;
		}

		// 방장과 클라이언트 모두 동기화 처리된 UI Timer로 업데이트 처리
		GOChecker->Multicast_UpdateExitToLobbyRemainTime(CurSecForUI);
		
		// 이전 TimerForUI 내용 갱신
		PrevExitToLobbyTimerForUI = CurSecForUI;
	}

	// 60초 카운트다운 완료 시 로비 이동
	if (ExitToLobbyTimer <= 0.f)
	{
		ExitToLobbyTimer = 999.f; // 중복 호출 방지
		OnExitToLobbyButtonClicked();
	}
}

void UC_GameOverWidget::OnExitToLobbyButtonClicked()
{
	PRINT_LOCAL(GetWorld(), "[UC_GameOverWidget::OnExitToLobbyButtonClicked]", FColor::Cyan, 10.f);
	
	if (bIsExiting) return;
	bIsExiting = true; // 중복 실행 방지

	/*// 스팀/온라인 세션 정리 후 스타트 레벨로 이동
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
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), LobbyLevel);*/

	// 오로지 서버 환경에서만 ExitToLobbyButton 기능 사용 가능 / 더블 체크
	AC_BasicPlayer* MyPlayer = LEVEL_MANAGER->GetLocalPlayer();
	if (!MyPlayer || !MyPlayer->HasAuthority()) return;
	
	// Lobby Level로 이동
	
	if (m_LobbyLevelName.IsEmpty()) return;

	const FString TravelURL = !m_LobbyLevelName.Contains(TEXT("?Listen")) ? m_LobbyLevelName + TEXT("?Listen") :
																			m_LobbyLevelName;
	
	GetWorld()->ServerTravel(TravelURL);
}
