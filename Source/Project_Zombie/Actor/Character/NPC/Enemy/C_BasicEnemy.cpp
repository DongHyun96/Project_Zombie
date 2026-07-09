// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Components/SkillComponent/C_EnemySkillComponent.h"
#include "Components/StatComponent/C_EnemyStatComponent.h"

AC_BasicEnemy::AC_BasicEnemy()
{
	// 스탯 컴포넌트 추가
	m_StatCom = CreateDefaultSubobject<UC_EnemyStatComponent>(TEXT("StatComponent"));

	// 스킬 컴포넌트 추가
	m_SkillCom = CreateDefaultSubobject<UC_EnemySkillComponent>(TEXT("SkillComponent"));
}

void AC_BasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (m_StatCom)
	{
		//GetCharacterMovement()->MaxWalkSpeed = m_StatCom->GetStat("MoveSpeed");
	}
}
