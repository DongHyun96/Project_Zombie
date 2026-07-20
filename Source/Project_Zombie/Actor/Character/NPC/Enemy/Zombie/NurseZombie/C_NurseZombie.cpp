// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NurseZombie.h"

#include "AudioDevice.h"
#include "NiagaraComponent.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/SphereComponent.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Utility/C_Util.h"


const uint8 AC_NurseZombie::s_HealTargetCountLimit = 3;

AC_NurseZombie::AC_NurseZombie()
	: Super(EZombieType::NurseZombie)
{
	PrimaryActorTick.bCanEverTick = false;

	m_HealingAuraCollider = CreateDefaultSubobject<USphereComponent>(TEXT("HealingAuraCollider"));
	m_HealingAuraCollider->SetupAttachment(GetRootComponent());
	
	m_HealingAuraEffectNG = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealingAuraNGComponent"));
	m_HealingAuraEffectNG->SetupAttachment(GetRootComponent());
}

void AC_NurseZombie::BeginPlay()
{
	Super::BeginPlay();
	ZOMBIE_MANAGER->AddNurseZombieToActivePool(this); // TODO : 이 라인 지워버리기 (Level 배치한 테스트용 처리 / 실질적인 Spawn 처리는 ZombieManager에서 할 것) 
	
	ToggleHealingAura(false);
}

void AC_NurseZombie::GetHealingAuraOverlappingEnemies(TArray<AActor*>& _OutOverlappingEnemies) const
{
	m_HealingAuraCollider->GetOverlappingActors(_OutOverlappingEnemies, AC_BasicEnemy::StaticClass());
}

void AC_NurseZombie::RemoveHealProjectileTarget(AC_BasicEnemy* _Target)
{
	const uint8 RemovedCount = m_HealProjectileTargets.Remove(_Target);
	if (RemovedCount == 0) return; // 제거된 대상이 없음

	// 제대로 제거가 되었음
	_Target->GetStatComponent()->OnCurHPReachedZeroDelegate.RemoveAll(this);
	_Target->GetStatComponent()->OnCurHPReachedFullDelegate.RemoveAll(this);
	
	// 제거 대상의 HealRequestRegisterCnt 하나 줄이기
	_Target->DecreaseHealRequestRegisterCount();
}

bool AC_NurseZombie::TryRegisterAsHealTarget(AC_BasicEnemy* _NewHealTarget)
{
	// Nurse끼리 Heal 가능 but 자기자신 HealTarget 잡기 x
	if (this == _NewHealTarget) return false;  
	
	if (m_HealProjectileTargets.Num() >= s_HealTargetCountLimit)
	{
		UC_Util::Print("Max TargetCount reached ", FColor::Red, 10.f);
		return false;
	}
	if (!_NewHealTarget || _NewHealTarget->GetStatComponent()->IsCurHPFull())	return false; // 새로 들어온 Target이 Valid하지 않거나, 이미 풀피인 상황
	
	if (m_HealProjectileTargets.Contains(_NewHealTarget))
	{
		UC_Util::Print("Already registered", FColor::Red, 10.f);	
		return false; // 이미 힐 Target으로 지정되어 있는 상황
	}

	// Dist Squared 값 Max (20m 이내의 적만 등록 가능하다)
	static const float MAX_DIST_LIMIT_SQR = 2000.f * 2000.f;
	if (FVector::DistSquared(GetActorLocation(), _NewHealTarget->GetActorLocation()) > MAX_DIST_LIMIT_SQR)
	{
		UC_Util::Print("MaxDist Reached", FColor::Red, 10.f);
		return false;
	}
	
	// 힐 타겟 등록 및 HealTarget의 체력이 모두 찼거나, 이미 사망한 경우에 대해 m_HealTargets에서 제거해버리는 Delegate 구독
	_NewHealTarget->GetStatComponent()->OnCurHPReachedZeroDelegate.AddUObject(this, &AC_NurseZombie::OnHealTargetDeadOrReachedFullHP);
	_NewHealTarget->GetStatComponent()->OnCurHPReachedFullDelegate.AddUObject(this, &AC_NurseZombie::OnHealTargetDeadOrReachedFullHP);

	m_HealProjectileTargets.Add(_NewHealTarget);

	UC_Util::Print("Heal target received!", FColor::Red, 10.f);
	
	return true;
}

void AC_NurseZombie::ToggleHealingAura(bool _Activate)
{
	if (_Activate)
	{
		m_HealingAuraCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		m_HealingAuraEffectNG->Activate(true);
	}
	else
	{
		m_HealingAuraCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_HealingAuraEffectNG->DeactivateImmediate();
	}
}

void AC_NurseZombie::OnHealTargetDeadOrReachedFullHP(AC_BasicCharacter* _HealTarget)
{
	if (!_HealTarget) return;
	if (_HealTarget->GetStatComponent()->IsCurHPFull()) UC_Util::Print("!!HealTarget Max HP Reached!!", FColor::MakeRandomColor(), 10.f);
	else UC_Util::Print("!!HealTarget Dead!!", FColor::MakeRandomColor(), 10.f);
	
	RemoveHealProjectileTarget(Cast<AC_BasicEnemy>(_HealTarget));
}

void AC_NurseZombie::OnHealSkillEnd(AC_BasicEnemy* _Enemy)
{
	ToggleHealingAura(false);
}

void AC_NurseZombie::OnDead(AC_BasicCharacter* _DeadCharacter)
{
	Super::OnDead(_DeadCharacter);
	
	// 자기 자신에게 등록되었던 모든 힐 대상 제거
	for (AC_BasicEnemy* HealTarget : m_HealProjectileTargets)
	{
		HealTarget->GetStatComponent()->OnCurHPReachedZeroDelegate.RemoveAll(this);
		HealTarget->GetStatComponent()->OnCurHPReachedFullDelegate.RemoveAll(this);
	
		// 제거 대상의 HealRequestRegisterCnt 하나 줄이기
		HealTarget->DecreaseHealRequestRegisterCount();
	}
	m_HealProjectileTargets.Empty();
}
