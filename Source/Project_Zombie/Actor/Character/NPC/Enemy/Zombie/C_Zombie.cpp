// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"


#include "../../../GlobalEnum.h"

AC_Zombie::AC_Zombie()
	: m_ZombieType(EZombieType::NormalZombie)
{
}

AC_Zombie::AC_Zombie(EZombieType _ZombieType)
	: m_ZombieType(_ZombieType)
{
}

void AC_Zombie::BeginPlay()
{
	Super::BeginPlay();
	
	// 팀 설정
	SetGenericTeamId(static_cast<uint8>(ETeamType::Enemy));
}

void AC_Zombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
