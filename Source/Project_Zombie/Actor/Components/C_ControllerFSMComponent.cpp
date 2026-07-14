// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ControllerFSMComponent.h"

#include "C_TurnInPlaceComponent.h"
#include "C_BasicPlayerAimComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/C_Util.h"


UC_ControllerFSMComponent::UC_ControllerFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UC_ControllerFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("From UC_ControllerFSMComponent::BeginPlay : OwnerPlayer init failed!", FColor::Red, 10.f);
		UE_LOG(LogTemp, Error, TEXT("From UC_ControllerFSMComponent::BeginPlay : OwnerPlayer init failed!"));
	}
	
	m_PlayerMovement = m_OwnerPlayer->GetCharacterMovement();
}

void UC_ControllerFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HandleFSMTransition();
	HandleFSMStates();
}

void UC_ControllerFSMComponent::OnTurnInPlaceFin()
{
	m_PlayerControllerRotState = EPlayerControllerRotState::IdleStopState;
}

void UC_ControllerFSMComponent::HandleFSMTransition()
{
	if (m_OwnerPlayer->IsFreeLook())
	{
		m_PlayerControllerRotState = EPlayerControllerRotState::FreeLookState;
		return;
	}

	const bool bIsMovingOrFalling = m_PlayerMovement->Velocity.SizeSquared() > 0.f; 
	
	// Idle -> Turn in place state
	// 0 360
	m_DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(m_OwnerPlayer->GetControlRotation(), m_OwnerPlayer->GetActorRotation()).Yaw;

	switch (m_PlayerControllerRotState)
	{
	case EPlayerControllerRotState::IdleStopState:
	{
		/* IdleStopState -> Moving State Transition */
		if (bIsMovingOrFalling)
		{
			m_PlayerControllerRotState = EPlayerControllerRotState::MovingState;
			return;
		}
		
		// TurnInPlace 처리 불가 각도
		if (FMath::Abs(m_DeltaYaw) <= 90.f) return;

		/* IdleStopState -> TurnInPlaceState Transition*/
		{
			m_PlayerControllerRotState = EPlayerControllerRotState::TurnInPlaceState;
			m_OwnerPlayer->GetTurnInPlaceComponent()->StartTurnInPlaceMotion(m_DeltaYaw);
		}
	}
		return;
	case EPlayerControllerRotState::TurnInPlaceState:
	{
		/* TurnInPlace -> Idle Stop state : TurnInPlace 끝나는 Notify 시점에 Transition 처리가 일어남 */
		
		/* TurnInPlace -> Moving State */
		if (bIsMovingOrFalling)
		{
			m_OwnerPlayer->GetTurnInPlaceComponent()->CancelTurnInPlaceMotionIfNecessary();
			m_PlayerControllerRotState = EPlayerControllerRotState::MovingState;
		}
	}
		return;
	case EPlayerControllerRotState::MovingState:
	{
		/* MovingState -> IdleStopState */
		if (!bIsMovingOrFalling)
			m_PlayerControllerRotState = EPlayerControllerRotState::IdleStopState;
	}
		return;
	case EPlayerControllerRotState::FreeLookState:
	{
		// 맨 위의 AnyState to FreeLook 처리가 들어오지 않은 상황이면, FreeLook 상태가 최초 해제된 상황
		// Idle Stop State로 우선 Default State로 처리
		m_PlayerControllerRotState = EPlayerControllerRotState::IdleStopState;
	}
		return;
	}
}

void UC_ControllerFSMComponent::HandleFSMStates()
{

	// 현재 플레이어가 조준(견착) 중이라면, 어떤 상태든 관계없이 컨트롤러 회전 동기화
	if (m_OwnerPlayer->GetAimComponent() && m_OwnerPlayer->GetAimComponent()->IsAiming())
	{
		m_OwnerPlayer->bUseControllerRotationYaw = true;  // 캐릭터가 카메라 방향을 정면으로 바라봄
		m_PlayerMovement->bUseControllerDesiredRotation = false;
		m_PlayerMovement->bOrientRotationToMovement = false; // 이동 방향으로 캐릭터가 돌지 않음
		return; // 조준 중일 때는 아래 일반 상태 설정을 무시하고 리턴
	}

	switch (m_PlayerControllerRotState)
	{
	case EPlayerControllerRotState::IdleStopState:
	{
		m_OwnerPlayer->bUseControllerRotationYaw        = false;
		m_PlayerMovement->bUseControllerDesiredRotation = false;
		m_PlayerMovement->bOrientRotationToMovement     = true;
	}
		return;
	case EPlayerControllerRotState::TurnInPlaceState:
	{
		m_OwnerPlayer->bUseControllerRotationYaw        = false;
		m_PlayerMovement->bUseControllerDesiredRotation = true;
		m_PlayerMovement->bOrientRotationToMovement     = false;
	}
		return;
	case EPlayerControllerRotState::MovingState:
	{
		m_OwnerPlayer->bUseControllerRotationYaw        = true;
		m_PlayerMovement->bUseControllerDesiredRotation = false;
		m_PlayerMovement->bOrientRotationToMovement     = false;
	}
		return;
	case EPlayerControllerRotState::FreeLookState:
	{
		m_OwnerPlayer->bUseControllerRotationYaw        = false;
		m_PlayerMovement->bUseControllerDesiredRotation = false;
		m_PlayerMovement->bOrientRotationToMovement     = false;		
	}
		return;
	}
}

