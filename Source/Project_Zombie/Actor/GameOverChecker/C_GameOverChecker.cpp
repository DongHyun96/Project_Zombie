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
	UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
	if (!MainHUD) return;
	
	MainHUD->GetInformWidget()->ShowMainInstruction("ACTIVATE THE NEXT POINT TOWERS !");
	
	// 혹시 모르니, GameStart 파넬 여기서 끄는 처리를 넣어줌
	MainHUD->GetInformWidget()->ToggleGameStartPanel(false);
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
	UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
	if (!MainHUD)
	{
		UC_Util::Print("From AC_GameOverChecker::Multicast_GameOver : MainHUD nullptr", FColor::Red, 10.f);
	}
	
	if (_PlayerWin) MainHUD->GetGameOverWidget()->ActivateWinningSequence();
	else MainHUD->GetGameOverWidget()->ActivateLoseSequence();
}
