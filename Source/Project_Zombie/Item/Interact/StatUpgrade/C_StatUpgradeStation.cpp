// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interact/StatUpgrade/C_StatUpgradeStation.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/UpgradeComponent/C_StatUpgradeComponent.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_UIManager.h"

AC_StatUpgradeStation::AC_StatUpgradeStation()
{
	m_UpgradeComp = CreateDefaultSubobject<UC_StatUpgradeComponent>(TEXT("PlayerStatUpgradeComp"));

}

void AC_StatUpgradeStation::RequestStatUpgrade(AC_BasicPlayer* InPlayer, const FName& UpStatName)
{
	if (!m_UpgradeComp)
	{
		AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(InPlayer->GetController());
		if (!PC) return;
		
		if (PC->HasAuthority())
			PC->FinishPlayerStatUpgrade();
		else
			PC->Client_FinishPlayerStatUpgrade();
		
		return;
	}
	PRINT_LOCAL(GetWorld(), "RequestItemUpgrade", FColor::Blue, 5.f);

	m_UpgradeComp->UpgradeStat(InPlayer, UpStatName);
}

void AC_StatUpgradeStation::BeginPlay()
{
	Super::BeginPlay();
}
