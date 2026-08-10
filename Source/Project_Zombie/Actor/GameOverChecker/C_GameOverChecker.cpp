// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameOverChecker.h"

#include "GameModeAndManager/C_UIManager.h"
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

void AC_GameOverChecker::Multicast_ShowMainInformConqueringPointTower_Implementation()
{
	AC_UIManager* UIManager = UI_MANAGER(GetWorld());
	if (!UIManager) return;
	
	UIManager->GetMainHUDWidget()->GetInformWidget()->ShowMainInstruction("CONQUER THE NEXT POINT TOWERS !");
	
	// 혹시 모르니, GameStart 파넬 여기서 끄는 처리를 넣어줌
	UIManager->GetMainHUDWidget()->GetInformWidget()->ToggleGameStartPanel(false);
}

void AC_GameOverChecker::Multicast_UpdateGameStartLeftTime_Implementation(int32 _LeftTime)
{
	AC_UIManager* UIManager = UI_MANAGER(GetWorld());
	if (!UIManager) return;

	if (!UIManager->GetMainHUDWidget()) return;
	
	UIManager->GetMainHUDWidget()->GetInformWidget()->ToggleGameStartPanel(true);
	UIManager->GetMainHUDWidget()->GetInformWidget()->UpdateGameStartLeftTime(_LeftTime);
}

void AC_GameOverChecker::Multicast_GameOver_Implementation(bool _PlayerWin)
{
	if (_PlayerWin) UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetGameOverWidget()->ActivateWinningSequence();
	else UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetGameOverWidget()->ActivateLoseSequence();
}
