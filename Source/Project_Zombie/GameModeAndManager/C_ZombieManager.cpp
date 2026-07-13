// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieManager.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"

UC_ZombieManager::UC_ZombieManager()
{
	// _C 없이 애셋 경로만
	static ConstructorHelpers::FClassFinder<AC_Zombie> NurseFinder(TEXT("/Game/DongHyun/Actor/Enemy/BP_NurseZombie"));

	if (NurseFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::NurseZombie, NurseFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> NormalFinder(TEXT("/Game/Harang/BP/Zombie/BP_NormalZombie"));

	if (NormalFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::NormalZombie, NormalFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> PoisonFinder(TEXT("/Game/Harang/BP/Zombie/BP_PoisonZombie"));

	if (PoisonFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::PoisonZombie, PoisonFinder.Class);
}

void UC_ZombieManager::OnWorldBeginPlay()
{
	// TODO : m_PoolCounts 만큼 좀비 pooling 해두기
}
