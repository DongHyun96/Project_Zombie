// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AttackTokenComponent.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

UC_AttackTokenComponent::UC_AttackTokenComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_AttackTokenComponent::BeginPlay()
{
	Super::BeginPlay();

}

bool UC_AttackTokenComponent::TryAcquireToken(AC_BasicEnemy* _Enemy)
{
	if(!IsValid(_Enemy))
		return false;

	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))
		return false;

	// 공격토큰은 서버에서만 관리
	if (!OwnerActor->HasAuthority())
		return false;

	RemoveAttackers();

	// 이미 토큰을 가지고 있으면 성공으로 처리
	if (m_CurrentAttackers.Contains(_Enemy))
		return true;

	// 최대 공격 가능 인원에 도달했다면
	if (m_CurrentAttackers.Num() >= m_MaxAttackCount)
		return false;

	m_CurrentAttackers.Add(_Enemy);

	return true;
}

void UC_AttackTokenComponent::ReleaseToken(AC_BasicEnemy* _Enemy)
{
}

bool UC_AttackTokenComponent::HasToken(const AC_BasicEnemy* _Enemy) const
{
	return false;
}

int32 UC_AttackTokenComponent::GetCurrentAttackCount() const
{
	return int32();
}

bool UC_AttackTokenComponent::CanAcquireToken() const
{
	return false;
}

void UC_AttackTokenComponent::RemoveAttackers()
{
}


