// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopZombie.h"


AC_CopZombie::AC_CopZombie()
	: Super(EZombieType::CopZombie)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AC_CopZombie::BeginPlay()
{
	Super::BeginPlay();
}

void AC_CopZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
