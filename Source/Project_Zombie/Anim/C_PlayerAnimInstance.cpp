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

	// HandState 갱신
	m_HandState = m_Character->GetHandState();
	
	// MoveState 갱신
	m_PlayerMoveState = m_Character->GetPlayerMoveState();

	// 속도 갱신
	FVector Velocity = m_Character->GetVelocity();
	m_GroundSpeed = Velocity.Size2D();

	// 속도가 10 이상일 때만 방향 갱신 (속도가 낮으면 방향이 불안정하게 나와서)
	if (m_GroundSpeed > 10.f)
		m_Direction = CalculateDirection(Velocity, m_Character->GetActorRotation());

	// 낙하, 수직 속도 갱신
	m_IsFall = m_MovementComponent->IsFalling();
	m_VerticalSpeed = m_MovementComponent->Velocity.Z;

	// 점프 입력 여부 갱신
	m_IsJumpInput = m_Character->IsJumpInput();
}
