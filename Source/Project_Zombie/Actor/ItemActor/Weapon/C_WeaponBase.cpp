// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WeaponBase.h"

AC_WeaponBase::AC_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AC_WeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AC_WeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

