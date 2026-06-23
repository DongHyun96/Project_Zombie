// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerAnimInstance.h"

#include "../Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"


void UC_PlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UC_PlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	m_Character = Cast<AC_BasicPlayer>(TryGetPawnOwner());

	if (nullptr != m_Character)
	{
		m_MovementComponent = m_Character->GetCharacterMovement();
	}
}

void UC_PlayerAnimInstance::NativeUpdateAnimation(float _DT)
{
	Super::NativeUpdateAnimation(_DT);

	if (nullptr == m_Character || nullptr == m_MovementComponent)
		return;

	FVector Velocity = m_Character->GetVelocity();
	m_GroundSpeed = Velocity.Size2D();

	if (m_GroundSpeed > 10.f)
		m_Direction = CalculateDirection(Velocity, m_Character->GetActorRotation());

	m_IsFall = m_MovementComponent->IsFalling();
	m_VerticalSpeed = m_MovementComponent->Velocity.Z;

	m_IsJumpInput = m_Character->IsJumpInput();
}
