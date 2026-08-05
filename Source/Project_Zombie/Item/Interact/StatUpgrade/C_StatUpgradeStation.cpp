// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interact/StatUpgrade/C_StatUpgradeStation.h"

#include "Actor/Components/UpgradeComponent/C_StatUpgradeComponent.h"
#include "GameModeAndManager/C_UIManager.h"

AC_StatUpgradeStation::AC_StatUpgradeStation()
{
	m_UpgradeComp = CreateDefaultSubobject<UC_StatUpgradeComponent>(TEXT("PlayerStatUpgradeComp"));

}

void AC_StatUpgradeStation::RequestStatUpgrade(AC_BasicPlayer* InPlayer, const FName& UpStatName)
{
	if (!m_UpgradeComp) return;

	PRINT_LOCAL(GetWorld(), "RequestItemUpgrade", FColor::Blue, 5.f);

	m_UpgradeComp->UpgradeItem(InPlayer, UpStatName);
}

void AC_StatUpgradeStation::BeginPlay()
{
	Super::BeginPlay();
}
