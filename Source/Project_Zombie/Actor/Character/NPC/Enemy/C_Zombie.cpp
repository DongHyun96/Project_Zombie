// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"
#include "C_EnemyStatComponent.h"

AC_Zombie::AC_Zombie()
{
	// 스탯 컴포넌트 추가
	m_StatCom = CreateDefaultSubobject<UC_EnemyStatComponent>(TEXT("StatComponent"));
}

void AC_Zombie::BeginPlay()
{
	Super::BeginPlay();
	
}

void AC_Zombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

