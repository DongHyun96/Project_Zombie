// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TurnInPlaceComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/C_Util.h"

UC_TurnInPlaceComponent::UC_TurnInPlaceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UC_TurnInPlaceComponent::BeginPlay()
{
	Super::BeginPlay();
	
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("From UC_TurnInPlaceComponent::BeginPlay : OwnerPlayer init failed!"));
		UC_Util::Print("From UC_TurnInPlaceComponent::BeginPlay : OwnerPlayer init failed!");
	}
}


void UC_TurnInPlaceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// HandleUpdateTurnInPlace(DeltaTime);
	
}

void UC_TurnInPlaceComponent::SetStrafeRotationToIdleStop()
{
	m_OwnerPlayer->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	m_OwnerPlayer->GetCharacterMovement()->bOrientRotationToMovement     = true;
	m_OwnerPlayer->bUseControllerRotationYaw                             = false;
}

bool UC_TurnInPlaceComponent::StartTurnInPlaceMotion(float _YawRotDelta)
{
	// HandState 및 Yaw Delta 값에 따른 Turn in place 몽타주 Animation 고르기
	// TODO : PoseState까지 넣은 상황이라면(Crouch) -> Crouch 모션 TurnInPlace 또한 처리를 해주어야 한다
	FTurnInPlaceMontages* TargetTurnInPlaceMontages = m_TurnInPlaceMontages.Find(m_OwnerPlayer->GetHandState());
	if (!TargetTurnInPlaceMontages)
	{
		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some TurnInPlace montage is missing", FColor::Red, 10.f);
		return false;
	}
	 
	UAnimMontage* TurnInPlaceMontageToPlay = (_YawRotDelta > 90.f) ? TargetTurnInPlaceMontages->TurnRightMontage : TargetTurnInPlaceMontages->TurnLeftMontage;
	if (!TurnInPlaceMontageToPlay)
	{
		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some TurnInPlace montage is missing", FColor::Red, 10.f);
		return false;
	}

	// 이미 해당 Animation 을 재생중인 상황
	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(TurnInPlaceMontageToPlay)) return false;

	// Full body TurnInPlace Montage 재생 ( TODO : 추후 Lower Body 처리도 넣어줄 것(총기 또는 Throwable 처리 시 필요))
	m_OwnerPlayer->PlayAnimMontage(TurnInPlaceMontageToPlay);
	return true;
}

void UC_TurnInPlaceComponent::HandleUpdateTurnInPlace(float DeltaTime)
{
	UCharacterMovementComponent* PlayerMovementComponent = m_OwnerPlayer->GetCharacterMovement(); 
	
	// TODO : TurnInPlace 할 수 없는 예외처리 더 있다면 더 해주어야 함
	if (PlayerMovementComponent->IsFalling())
	{
		
		return;
	}
	
	// 0 360
	const float Delta = UKismetMathLibrary::NormalizedDeltaRotator(m_OwnerPlayer->GetControlRotation(), m_OwnerPlayer->GetActorRotation()).Yaw;
	
	if (-90.f <= Delta && Delta <= 90.f)
	{
		// Turn in place를 처리할 수 없는 각도
		return;
	}
	
	// Controller settings -> 이걸 이제, ControllerFSM에서 일괄 처리를 할 예정
	PlayerMovementComponent->bUseControllerDesiredRotation = true;
	PlayerMovementComponent->bOrientRotationToMovement     = false;
	
	// HandState 및 Yaw Delta 값에 따른 Turn in place 몽타주 Animation 고르기
	// TODO : PoseState까지 넣은 상황이라면(Crouch) -> Crouch 모션 TurnInPlace 또한 처리를 해주어야 한다
	FTurnInPlaceMontages* TargetTurnInPlaceMontages = m_TurnInPlaceMontages.Find(m_OwnerPlayer->GetHandState());
	if (!TargetTurnInPlaceMontages) return;
	 
	UAnimMontage* TurnInPlaceMontageToPlay = (Delta > 90.f) ? TargetTurnInPlaceMontages->TurnRightMontage : TargetTurnInPlaceMontages->TurnLeftMontage;
	if (!TurnInPlaceMontageToPlay) return;

	// 이미 해당 Animation 을 재생중인 상황
	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(TurnInPlaceMontageToPlay)) return;

	// Full body TurnInPlace Montage 재생 ( TODO : 추후 Lower Body 처리도 넣어줄 것(총기 또는 Throwable 처리 시 필요))
	m_OwnerPlayer->PlayAnimMontage(TurnInPlaceMontageToPlay);
}

void UC_TurnInPlaceComponent::CancelTurnInPlaceMotionIfNecessary()
{
	// Turn In Place중 움직이면 Turn In place 몽타주 끊고 해당 방향으로 바로 움직이게 하기
	
	FTurnInPlaceMontages* TargetTurnInPlaceMontages = m_TurnInPlaceMontages.Find(m_OwnerPlayer->GetHandState());
	if (!TargetTurnInPlaceMontages) return;
	
	UAnimMontage* RightMontage	= m_TurnInPlaceMontages[m_OwnerPlayer->GetHandState()].TurnRightMontage;
	UAnimMontage* LeftMontage	= m_TurnInPlaceMontages[m_OwnerPlayer->GetHandState()].TurnLeftMontage;
	UAnimInstance* AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();

	if (AnimInstance->Montage_IsPlaying(RightMontage))
	{
		// SetStrafeRotationToIdleStop();
		AnimInstance->Montage_Stop(0.2f, RightMontage);
	}

	if (AnimInstance->Montage_IsPlaying(LeftMontage))
	{
		// SetStrafeRotationToIdleStop();
		AnimInstance->Montage_Stop(0.2f, LeftMontage);
	}

	// TODO : Lower body part도 확인 (만약 넣는다면)
}