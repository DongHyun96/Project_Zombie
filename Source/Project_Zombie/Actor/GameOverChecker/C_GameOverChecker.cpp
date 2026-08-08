// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameOverChecker.h"

#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/GameOverWidget/C_GameOverWidget.h"


AC_GameOverChecker::AC_GameOverChecker()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	bAlwaysRelevant = true;
}

void AC_GameOverChecker::BeginPlay()
{
	Super::BeginPlay();
	
	PRINT_LOCAL(GetWorld(), "GameOverChecker SPawned", FColor::Red, 10.f);
}


void AC_GameOverChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_GameOverChecker::Multicast_GameOver_Implementation(bool _PlayerWin)
{
	if (_PlayerWin) UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetGameOverWidget()->ActivateWinningSequence();
	else UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetGameOverWidget()->ActivateLoseSequence();
}
