// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
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
	
	m_HealedEffectNGComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealedEffectNGComponent"));
	m_HealedEffectNGComponent->SetAutoDestroy(false); // NS의 Loop가 Once일 경우, NGComponent Destroy 처리 방지
	m_HealedEffectNGComponent->SetupAttachment(GetRootComponent());
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> HealedEffect
	(TEXT("/Script/Niagara.NiagaraSystem'/Game/DongHyun/Effect/EnemyHealed.EnemyHealed'"));
	
	if (HealedEffect.Succeeded())
		m_HealedEffectNGComponent->SetAsset(HealedEffect.Object.Get());
}

void AC_BasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 죽었을 때 처리할 함수 Delegate 구독 처리
	m_StatComponent->OnCurHPReachedZeroDelegate.AddUObject(this, &AC_BasicEnemy::OnDead);

	// IncreaseCurHP 정상 처리 시(힐 받은 처리로 판단) -> 힐 받은 Effect 활성화 함수 Delegate 구독 처리
	m_StatComponent->OnIncreaseCurHPDelegate.AddUObject(this, &AC_BasicEnemy::OnHPIncreased);

	// HealEffect 재생 속도 조절
	m_HealedEffectNGComponent->SetCustomTimeDilation(2.f);
	m_HealedEffectNGComponent->DeactivateImmediate();
	
	// 바닥면으로 위치 맞추기
	m_HealedEffectNGComponent->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
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

void AC_BasicEnemy::OnHPIncreased(AC_BasicCharacter* _HPIncreasedCharacter)
{
	// 이미 HealedEffect 재생중인 경우
	if (m_HealedEffectNGComponent->IsActive()) return;
	m_HealedEffectNGComponent->Activate(true);
}

void AC_BasicEnemy::OnDead(AC_BasicCharacter* _DeadCharacter)
{
	m_HealedEffectNGComponent->DeactivateImmediate();
	// TODO : Dead에 필요한 처리가 더 필요하다면 여기서 이어서 처리해줄 것(ex 랙돌 처리 등)
}
