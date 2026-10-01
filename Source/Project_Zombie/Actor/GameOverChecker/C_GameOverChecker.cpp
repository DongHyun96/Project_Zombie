// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameOverChecker.h"

#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/GameOverWidget/C_GameOverWidget.h"
#include "UI/MainHUD/InformWidget/C_InformWidget.h"
#include "Utility/C_Util.h"


AC_GameOverChecker::AC_GameOverChecker()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	bAlwaysRelevant = true;
}

void AC_GameOverChecker::BeginPlay()
{
	Super::BeginPlay();
}

void AC_GameOverChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_GameOverChecker::Multicast_UpdateRemainTime_Implementation(int32 _RemainTime)
{
	UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
	if (!MainHUD) return;

	MainHUD->GetInformWidget()->SetTimeRemainInfo(_RemainTime);
}

void AC_GameOverChecker::Multicast_ShowMainInformConqueringPointTower_Implementation(uint8 _CurSeqPointTowerCount, int32 _CurSeqRemainTime)
{
	UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
	if (!MainHUD) return;
	
	MainHUD->GetInformWidget()->ShowMainInstruction("ACTIVATE THE NEXT POINT TOWERS !");
	
	// 혹시 모르니, GameStart 파넬 여기서 끄는 처리를 넣어줌
	MainHUD->GetInformWidget()->ToggleGameStartPanel(false);
	
	// 개수에 맞게끔 PointTower 점령 % 정보 활성화(InformWidget 좌상단)
	MainHUD->GetInformWidget()->ShowTowerConqueredInfo(_CurSeqPointTowerCount);
	
	// RemainTime 정보 활성화
	MainHUD->GetInformWidget()->ToggleTimeRemainInfo(true, _CurSeqRemainTime);
}

void AC_GameOverChecker::Multicast_UpdateGameStartLeftTime_Implementation(int32 _LeftTime)
{
	UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
    if (!MainHUD) return;

	MainHUD->GetInformWidget()->ToggleGameStartPanel(true);
	MainHUD->GetInformWidget()->UpdateGameStartLeftTime(_LeftTime);
}

void AC_GameOverChecker::Multicast_GameOver_Implementation(bool _PlayerWin)
{
	m_bGameOver = true;
	
	if (HasAuthority()) POINT_TOWER_MANAGER(this)->ClearCurSeqLeftTimerHandle(); // GameOver 시, 현재 Sequence의 LeftTime 재는 Timer clear 처리 (서버 쪽만 하면된다)
	
	UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
	if (!MainHUD)
	{
		UC_Util::Print("From AC_GameOverChecker::Multicast_GameOver : MainHUD nullptr", FColor::Red, 10.f);
	}
	
	if (_PlayerWin) MainHUD->GetGameOverWidget()->ActivateWinningSequence();
	else MainHUD->GetGameOverWidget()->ActivateLoseSequence();
	
	MainHUD->GetInformWidget()->HideAllCurSequenceInfo();
}
