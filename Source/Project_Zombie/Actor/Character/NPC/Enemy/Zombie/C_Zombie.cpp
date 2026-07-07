// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"

#include "../C_EnemyStatComponent.h"
#include "../../../GlobalEnum.h"

AC_Zombie::AC_Zombie()
{
	// 팀 설정
	SetGenericTeamId((uint8)ETeamType::Enemy);
}

void AC_Zombie::BeginPlay()
{
	Super::BeginPlay();
	
}

void AC_Zombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


