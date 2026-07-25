// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WeaponBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Net/UnrealNetwork.h"
#include "UI/MainHUD/C_GameMainHUD.h"

AC_WeaponBase::AC_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true; // Tick 필요 없으면 끄기.

	SetReplicates(true);
	
	ItemLinkComp = CreateDefaultSubobject<UC_ItemLinkComponent>(TEXT("ItemLinkComp"));
}

void AC_WeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AC_WeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_WeaponBase::OnRep_OwnerPlayer()
{
}

void AC_WeaponBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 리플리케이트 하고싶은 멤버를 등록 여기서
	DOREPLIFETIME(AC_WeaponBase, m_OwnerPlayer);
}

