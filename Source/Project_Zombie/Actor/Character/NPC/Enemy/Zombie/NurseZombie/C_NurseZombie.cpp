// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NurseZombie.h"

#include "NiagaraComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Utility/C_Util.h"


const uint8 AC_NurseZombie::s_HealTargetCountLimit = 3;

AC_NurseZombie::AC_NurseZombie()
	: Super(EZombieType::NurseZombie)
{
	PrimaryActorTick.bCanEverTick = false;
	
	m_HealingAuraEffectNG = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealingAuraNGComponent"));
	m_HealingAuraEffectNG->SetupAttachment(GetRootComponent());
}

void AC_NurseZombie::BeginPlay()
{
	Super::BeginPlay();
	ZOMBIE_MANAGER->AddNurseZombieToActivePool(this); // TODO : 이 라인 지워버리기 (Level 배치한 테스트용 처리 / 실질적인 Spawn 처리는 ZombieManager에서 할 것) 
}

bool AC_NurseZombie::TryRegisterAsHealTarget(AC_BasicEnemy* _NewHealTarget)
{
	if (m_HealTargets.Num() >= s_HealTargetCountLimit)
	{
		UC_Util::Print("Max TargetCount reached ", FColor::Red, 10.f);
		return false;
	}
	if (!_NewHealTarget || _NewHealTarget->GetStatComponent()->IsCurHPFull())	return false; // 새로 들어온 Target이 Valid하지 않거나, 이미 풀피인 상황S
	
	if (m_HealTargets.Contains(_NewHealTarget))
	{
		UC_Util::Print("Already registered", FColor::Red, 10.f);	
		return false; // 이미 힐 Target으로 지정되어 있는 상황
	}

	// Dist Squared 값 Max (20m 이내의 적만 등록 가능하다)
	static const float MAX_DIST_LIMIT_SQR = 2000.f * 2000.f;
	/*if (FVector::DistSquared(GetActorLocation(), _NewHealTarget->GetActorLocation()) > MAX_DIST_LIMIT_SQR) // TODO : 이 주석 풀기 (For testing)
	{
		UC_Util::Print("MaxDist Reached", FColor::Red, 10.f);
		return false;
	}*/
	
	// 힐 타겟 등록 및 HealTarget의 체력이 모두 찼거나, 이미 사망한 경우에 대해 m_HealTargets에서 제거해버리는 Delegate 구독
	_NewHealTarget->GetStatComponent()->OnCurHPReachedZeroDelegate.AddUObject(this, &AC_NurseZombie::OnHealTargetDeadOrReachedFullHP);
	_NewHealTarget->GetStatComponent()->OnCurHPReachedFullDelegate.AddUObject(this, &AC_NurseZombie::OnHealTargetDeadOrReachedFullHP);

	m_HealTargets.Add(_NewHealTarget);

	UC_Util::Print("Heal target received!", FColor::Red, 10.f);
	
	return true;
}

void AC_NurseZombie::RemoveHealTarget(AC_BasicEnemy* _HealTarget)
{
	if (!_HealTarget) return;
	
	_HealTarget->GetStatComponent()->OnCurHPReachedZeroDelegate.RemoveAll(this);
	_HealTarget->GetStatComponent()->OnCurHPReachedFullDelegate.RemoveAll(this);
	
	m_HealTargets.Remove(_HealTarget);
}

void AC_NurseZombie::OnHealTargetDeadOrReachedFullHP(AC_BasicCharacter* _HealTarget)
{
	RemoveHealTarget(Cast<AC_BasicEnemy>(_HealTarget));
}
