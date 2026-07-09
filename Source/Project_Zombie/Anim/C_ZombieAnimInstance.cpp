// Fill out your copyright notice in the Description page of Project Settings.

#include "C_ZombieAnimInstance.h"

#include "../Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Actor/Character/NPC/Enemy/C_EnemySkillComponent.h"

void UC_ZombieAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UC_ZombieAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	m_Zombie = Cast<AC_Zombie>(TryGetPawnOwner());

	if (nullptr != m_Zombie)
	{
		m_MovementComponent = m_Zombie->GetCharacterMovement();
	}
}

void UC_ZombieAnimInstance::NativeUpdateAnimation(float _DT)
{
	Super::NativeUpdateAnimation(_DT);

	if (nullptr == m_Zombie || nullptr == m_MovementComponent)
		return;

	FVector Valocity = m_Zombie->GetVelocity();

	m_GroundSpeed = Valocity.Size2D();

	if (10.f < m_GroundSpeed)
		m_Direction = CalculateDirection(Valocity, m_Zombie->GetActorRotation());

	m_VerticalSpeed = m_MovementComponent->Velocity.Z;
}

void UC_ZombieAnimInstance::AnimNotify_SkillEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("AN_SkillEnd"));

	m_Zombie->GetSkillComponent()->EndSkill();
}
