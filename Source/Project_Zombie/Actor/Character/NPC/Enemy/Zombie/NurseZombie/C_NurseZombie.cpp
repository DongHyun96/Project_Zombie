// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NurseZombie.h"

#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"


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
	
	static float Timer = 0.f;
	Timer += DeltaTime;
	if (Timer > 1.f)
	{
		Timer -= 1.f;

		for (AC_BasicEnemy* HealTarget : m_HealTargets)
			ZOMBIE_MANAGER->SpawnHealingProjectile(GetActorLocation() + FVector::UnitZ() * 100.f, FVector::UnitZ(), this, HealTarget, 30.f);
	}
}
