// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WeaponBase.h"

#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"

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

