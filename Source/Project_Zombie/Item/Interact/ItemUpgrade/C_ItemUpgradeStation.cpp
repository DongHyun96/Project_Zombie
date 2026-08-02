// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interact/ItemUpgrade/C_ItemUpgradeStation.h"

#include "Actor/Components/UpgradeComponent/C_ItemUpgradeComponent.h"
#include "GameModeAndManager/C_UIManager.h"

AC_ItemUpgradeStation::AC_ItemUpgradeStation()
{
	m_UpgradeComp = CreateDefaultSubobject<UC_ItemUpgradeComponent>(TEXT("ItemUpgradeComp"));
}

void AC_ItemUpgradeStation::BeginPlay()
{
	Super::BeginPlay();
}

void AC_ItemUpgradeStation::RequestItemUpgrade(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat)
{
	if (!m_UpgradeComp) return;

	PRINT_LOCAL(GetWorld(), "RequestItemUpgrade", FColor::Blue, 5.f);

	m_UpgradeComp->UpgradeItem(InPlayer, InItemIndex, TargetStat);
}