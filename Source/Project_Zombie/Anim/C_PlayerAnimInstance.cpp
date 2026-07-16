// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerAnimInstance.h"

#include "../Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "../Actor/Character/Player/C_BasicPlayer.h"
#include "../Actor/Components/C_EquippedComponent.h"
#include "../Actor/Components/C_ControllerFSMComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	// Crouch 여부 갱신
	m_IsCrouch = m_Character->IsCrouching();

	// 속도 갱신
	FVector Velocity = m_Character->GetVelocity();
	m_GroundSpeed = Velocity.Size2D();

	// 속도가 10 이상일 때만 방향 갱신 (속도가 낮으면 방향이 불안정하게 나와서)
	if (m_GroundSpeed > 10.f)
		m_Direction = CalculateDirection(Velocity, m_Character->GetActorRotation());

	m_Alpha = (m_GroundSpeed > 160.f) ? 0.0f : 1.0f;

	// 낙하, 수직 속도 갱신
	m_IsFall = m_MovementComponent->IsFalling();
	m_VerticalSpeed = m_MovementComponent->Velocity.Z;

	// 점프 입력 여부 갱신
	m_IsJumpInput = m_Character->IsJumpInput();

	// 에디터 화면일 때는 아래 로직을 아예 실행하지 않고 리턴.
	if (GetWorld() && GetWorld()->WorldType == EWorldType::EditorPreview)
		return;

	if (!m_Character) return;

	if (AC_GunBase* CurrentGun = Cast<AC_GunBase>(m_Character->GetEquippedComponent()->GetCurWeapon()))
	{
		if (USkeletalMeshComponent* WeaponMesh = CurrentGun->GetWeaponMesh())
		{
			m_LeftHandIKTransform = WeaponMesh->GetSocketTransform(TEXT("IK_Socket_LeftHand"), RTS_World).GetLocation();;
		}
	}

	// 캐릭터의 기본 조준 회전값(Aim Rotation) 가져오기
	FRotator AimRotation = m_Character->GetBaseAimRotation();

	// 캐릭터 자체의 회전값 가져오기
	FRotator ActorRotation = m_Character->GetActorRotation();

	// 캐릭터 회전과 조준 회전의 차이 계산
	// 조준 각도가 캐릭터 정면을 기준으로 얼마나 위/아래로 꺾였는지 계산
	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);

	// 피치 값 할당
	m_Pitch = DeltaRotation.Pitch;
	// 턴 인 플레이스의 야값 할당
	// m_Yaw = m_Character->GetControllerFSM()->GetDeltaYaw();
	m_Yaw = DeltaRotation.Yaw;
	
}
