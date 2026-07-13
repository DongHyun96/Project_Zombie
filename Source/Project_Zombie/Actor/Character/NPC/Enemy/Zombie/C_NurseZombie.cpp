// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NurseZombie.h"


AC_NurseZombie::AC_NurseZombie()
	: Super(EZombieType::NurseZombie)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AC_NurseZombie::BeginPlay()
{
	Super::BeginPlay();
}

void AC_NurseZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
