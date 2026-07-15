// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Components/SkillComponent/C_EnemySkillComponent.h"
#include "Components/StatComponent/C_EnemyStatComponent.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Utility/C_Util.h"
#include "Zombie/NurseZombie/C_NurseZombie.h"

AC_BasicEnemy::AC_BasicEnemy()
{
	// 스탯 컴포넌트 추가
	m_StatComponent = CreateDefaultSubobject<UC_EnemyStatComponent>(TEXT("StatComponent"));

	// 스킬 컴포넌트 추가
	m_SkillCom = CreateDefaultSubobject<UC_EnemySkillComponent>(TEXT("SkillComponent"));
}

float AC_BasicEnemy::TakeDamage
(
	float				_DamageAmount,
	FDamageEvent const& _DamageEvent,
	AController*		_EventInstigator,
	AActor*				_DamageCauser
)
{
	const float DamageAmount = Super::TakeDamage(_DamageAmount, _DamageEvent, _EventInstigator, _DamageCauser);
	UC_Util::Print("Zombie Damaged", FColor::Red, 10.f);

	// 현재 생명력 Ratio 50% ~ 70% 랜덤 수치 이하면, 가능한 힐러 좀비에게 힐 요청 시도
	// TODO : 이거 요청 빈도가 너무 높으면 여기서 병목 생길수도 있음 -> 추후 최적화할 때 고려할 것
	if (m_StatComponent->GetCurHPRatio() < FMath::RandRange(0.5f, 0.7f))
	{
		for (AC_NurseZombie* ActiveNurse : ZOMBIE_MANAGER->GetActiveNurseZombies())
			if (ActiveNurse->TryRegisterAsHealTarget(this)) break; // Nurse HealTarget에 정상 등록 처리됨 (Available한 Nurse가 없을 수도 있음)
	}
	
	return DamageAmount;
}

void AC_BasicEnemy::BeginPlay()
{
	Super::BeginPlay();
}
