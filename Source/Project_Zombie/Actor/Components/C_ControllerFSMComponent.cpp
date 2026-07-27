// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ControllerFSMComponent.h"

#include "C_TurnInPlaceComponent.h"
#include "C_BasicPlayerAimComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Utility/C_Util.h"


UC_ControllerFSMComponent::UC_ControllerFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
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

	// ControllerFSM 주최는 오로지 LocallyControlled Player 자신
	if (!m_OwnerPlayer->IsLocallyControlled()) return;
	
	HandleFSMTransition();
}

void UC_ControllerFSMComponent::OnTurnInPlaceFin()
{
	if (m_OwnerPlayer->IsLocallyControlled())
		SetControllerRotState(EPlayerControllerRotState::IdleStopState);
}

void UC_ControllerFSMComponent::HandleFSMTransition()
{
	if (m_OwnerPlayer->IsFreeLook())
	{
		SetControllerRotState(EPlayerControllerRotState::FreeLookState);
		return;
	}

	const bool bIsMovingOrFalling = m_PlayerMovement->Velocity.SizeSquared() > 0.f; 

	switch (m_PlayerControllerRotState)
	{
	case EPlayerControllerRotState::IdleStopState:
	{
		/* IdleStopState -> Moving State Transition */
		if (bIsMovingOrFalling)
		{
			SetControllerRotState(EPlayerControllerRotState::MovingState);
			return;
		}
		
		// Idle -> Turn in place state
		// 0 360
		const float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(m_OwnerPlayer->GetControlRotation(), m_OwnerPlayer->GetActorRotation()).Yaw;
		
		// TurnInPlace 처리 불가 각도
		if (FMath::Abs(DeltaYaw) <= 90.f) return;

		/* IdleStopState -> TurnInPlaceState Transition*/
		{
			SetControllerRotState(EPlayerControllerRotState::TurnInPlaceState);
			m_OwnerPlayer->GetTurnInPlaceComponent()->StartTurnInPlaceMotion(DeltaYaw > 90.f);
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
			SetControllerRotState(EPlayerControllerRotState::MovingState);
		}
	}
		return;
	case EPlayerControllerRotState::MovingState:
	{
		/* MovingState -> IdleStopState */
		if (!bIsMovingOrFalling)
			SetControllerRotState(EPlayerControllerRotState::IdleStopState);
	}
		return;
	case EPlayerControllerRotState::FreeLookState:
	{
		// 맨 위의 AnyState to FreeLook 처리가 들어오지 않은 상황이면, FreeLook 상태가 최초 해제된 상황
		// Idle Stop State로 우선 Default State로 처리
		SetControllerRotState(EPlayerControllerRotState::IdleStopState);
	}
		return;
	}
}

void UC_ControllerFSMComponent::SetControllerRotState(EPlayerControllerRotState _NewRotState)
{
	if (m_PlayerControllerRotState == _NewRotState) return;
	
	m_PlayerControllerRotState = _NewRotState;

	// 새로 반영된 RotValue에 따른 각 Rot 값 수정
	SetEachRotValueByCurState();

	// 서버 환경에서의 로컬 플레이어를 제외한 나머지 플레이어들은 동기화 요청을 보내주어야 한다 (서버 쪽 로컬 플레이어는 알아서 replicate 처리가 됨)
	if (m_OwnerPlayer->IsLocallyControlled() && !m_OwnerPlayer->HasAuthority())
		Server_SetControllerRotState(_NewRotState);
}

void UC_ControllerFSMComponent::Server_SetControllerRotState_Implementation(EPlayerControllerRotState _NewRotState)
{
	m_PlayerControllerRotState = _NewRotState;
	SetEachRotValueByCurState(); // 서버도 클라이언트와 똑같은 Rotation 플래그 설정값을 가지게끔 처리
}

void UC_ControllerFSMComponent::SetEachRotValueByCurState()
{
	if (!m_OwnerPlayer) return;
	
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

void UC_ControllerFSMComponent::OnRep_PlayerControllerRotState()
{
	SetEachRotValueByCurState(); // Rotation 값 동기화 처리
}

void UC_ControllerFSMComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UC_ControllerFSMComponent, m_PlayerControllerRotState);
}

