// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PoseColliderHandlerComponent.h"

#include "Engine/Engine.h"

#include "../Character/Player/C_BasicPlayer.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"

#include "Utility/C_Util.h"

UC_PoseColliderHandlerComponent::UC_PoseColliderHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 자세 전환 중에만 Tick 활성화
	PrimaryComponentTick.bStartWithTickEnabled = true;
}


void UC_PoseColliderHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Player = Cast<AC_BasicPlayer>(GetOwner());

	if (!m_Player)
	{
		SetComponentTickEnabled(false);
		return;
	}

	m_CapsuleComponent = m_Player->GetCapsuleComponent();

	m_MeshComponent = m_Player->GetMesh();

	if (!m_CapsuleComponent || !m_MeshComponent)
	{
		SetComponentTickEnabled(false);
		return;
	}

	m_CharacterMovementComponent = m_Player->GetCharacterMovement();

	// CharacterMovementComponent 의 UpdatedComponent 가 CapsuleComponent 가 아닌 경우 경고 출력
	if (m_CharacterMovementComponent->UpdatedComponent != m_CapsuleComponent)
	{
		UC_Util::Print("[UC_PoseColliderHandlerComponent::BeginPlay] Not Capsule");
	}

	// 현재 Capsule 크기를 기준으로 저장
	m_CapsuleComponent->GetUnscaledCapsuleSize(m_StandRadius, m_StandHalfHeight);

	// 웅크리기 최소 높이
	constexpr float DefaultCrouchHalfHeight = 60.f;
	if (m_CrouchHalfHeight <= DefaultCrouchHalfHeight)
	{
		m_CrouchHalfHeight = DefaultCrouchHalfHeight;
	}

	// 현재 Mesh 상대 위치를 저장
	m_StandMeshRelativeLocation = m_MeshComponent->GetRelativeLocation();

	m_bIsCrouched = false;
	m_bTargetCrouched = false;
	m_bIsTransitioning = false;

	// CharacterMovementComponent 의 Tick 이 먼저 실행되도록 설정
	AddTickPrerequisiteComponent
	(
		m_CharacterMovementComponent
	);

	// Tick 은 자세 전환 중에만 활성화
	SetComponentTickEnabled(false);
}


void UC_PoseColliderHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_bIsTransitioning || !m_Player || !m_CapsuleComponent || !m_CharacterMovementComponent)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 서버만 캐릭터가 일어날 수 있는지 판단
	if (!m_bTargetCrouched && !CanStand() && m_Player->HasAuthority())
	{
		m_bTargetCrouched = true;
	}

	// 어떤 자세로 전환할지 결정
	const float TargetHalfHeight = m_bTargetCrouched ? m_CrouchHalfHeight : m_StandHalfHeight;

	// 현재 Capsule Half Height
	const float CurrentHalfHeight = m_CapsuleComponent->GetUnscaledCapsuleHalfHeight();

	// 전환 시간 안에 전환되도록 보간
	const float TotalHeightDifference = FMath::Abs(m_StandHalfHeight - m_CrouchHalfHeight);

	// 전환 속도 계산
	const float InterpSpeed = TotalHeightDifference / m_TransitionDuration;

	// 현재 높이를 목표 높이 방향으로 일정하게 변경
	const float NewHalfHeight =
		FMath::FInterpConstantTo
		(
			CurrentHalfHeight,
			TargetHalfHeight,
			DeltaTime,
			InterpSpeed
		);

	ApplyCapsuleHalfHeight(NewHalfHeight);

	if (FMath::IsNearlyEqual(NewHalfHeight, TargetHalfHeight))
	{
		FinishTransition();
	}
}

bool UC_PoseColliderHandlerComponent::SetCrouched(bool _bIsCrouched)
{
	// 전환 중에는 중복 요청 방지
	if (m_bIsTransitioning)
	{
		return false;
	}

	// 이미 요청한 자세와 동일하면 무시
	if (m_bIsCrouched == _bIsCrouched)
	{
		return false;
	}

	// 땅 위에 서있지 않으면 무시
	if (!m_CharacterMovementComponent->IsMovingOnGround())
	{
		return false;
	}

	// 일어서는 자세로 변환하려고 했는데 위에 공간이 없어서 못일어나는 경우 무시
	if (!_bIsCrouched && !CanStand())
	{
		if (m_Player && m_Player->IsLocallyControlled())
			if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
				MainHUD->AddPlayerWarningLog("STANDING BLOCKED");
		
		return false;
	}

	StartCrouchTransition(_bIsCrouched);

	return true;
}

void UC_PoseColliderHandlerComponent::StartCrouchTransition(bool _bIsCrouched)
{
	// 이미 같은 방향으로 전환 중
	if (m_bIsTransitioning && m_bTargetCrouched == _bIsCrouched)
	{
		return;
	}

	// 이미 전환 중이라면 무시
	if (m_bIsTransitioning && m_bIsCrouched == _bIsCrouched)
	{
		return;
	}

	m_bTargetCrouched = _bIsCrouched;
	m_bIsTransitioning = true;

	// 자세 전환 시작 (Tick 활성화)
	SetComponentTickEnabled(true);
}

void UC_PoseColliderHandlerComponent::ApplyRemotePose(bool _bIsCrouched)
{
	const float TargetHalfHeight = _bIsCrouched ? m_CrouchHalfHeight : m_StandHalfHeight;

	m_CapsuleComponent->SetCapsuleSize(m_StandRadius, TargetHalfHeight, false);

	FVector TargetMeshLocation = m_StandMeshRelativeLocation;

	if (_bIsCrouched)
	{
		TargetMeshLocation.Z += (m_StandHalfHeight - m_CrouchHalfHeight);
	}

	m_MeshComponent->SetRelativeLocation(
		TargetMeshLocation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	m_Player->CacheInitialMeshOffset
	(
		TargetMeshLocation,
		m_MeshComponent->GetRelativeRotation()
	);

	// 컴포넌트 내부 상태도 현재 자세와 맞춤
	m_bIsCrouched = _bIsCrouched;
	m_bTargetCrouched = _bIsCrouched;
	m_bIsTransitioning = false;

	SetComponentTickEnabled(false);

	m_CapsuleComponent->UpdateOverlaps();
}

bool UC_PoseColliderHandlerComponent::CanStand() const
{
	// 현재 Capsule Scale
	const float CapsuleScale = m_CapsuleComponent->GetShapeScale();

	// 현재 Capsule Scale Half Height
	const float CurrentScaledHalfHeight = m_CapsuleComponent->GetScaledCapsuleHalfHeight();

	const float StandScaledHalfHeight = m_StandHalfHeight * CapsuleScale;

	const float StandScaledRadius = m_StandRadius * CapsuleScale;

	// 현재 높이에서 서 있는 높이로 전환할 때 필요한 높이 차이
	const float HeightDifference = StandScaledHalfHeight - CurrentScaledHalfHeight;

	// Capsule 바닥을 유지하면서 일어나기 위해 중심이 위로 이동
	const FVector StandCapsuleLocation = m_CapsuleComponent->GetComponentLocation() + FVector::UpVector * HeightDifference;

	// 실제 Stand Capsule 크기 검사용 Capsule
	const FCollisionShape StandCapsuleShape = FCollisionShape::MakeCapsule(StandScaledRadius, StandScaledHalfHeight);

	// 충돌 체크를 위한 QueryParams 설정
	FCollisionQueryParams QueryParams
	(
		SCENE_QUERY_STAT(PlayerCanStand),
		false,
		m_Player
	);

	// 플레이어에 부착된 무기들 무시
	TArray<AActor*> AttachedActors;
	m_Player->GetAttachedActors(AttachedActors);
	QueryParams.AddIgnoredActors(AttachedActors);

	// 서 있는 크기의 StandCapsuleShape 를 StandCapsuleLocation 위치에 배치했을 때 충돌이 발생하는지 확인
	const bool bBlocked = GetWorld()->OverlapBlockingTestByChannel
	(
		StandCapsuleLocation,
		FQuat::Identity,
		ECC_Pawn,					// 충돌 채널 설정 
		StandCapsuleShape,
		QueryParams
	);

	// 막힌 경우 false 반환, 막히지 않은 경우 true 반환
	return !bBlocked;
}

void UC_PoseColliderHandlerComponent::ApplyCapsuleHalfHeight(float _NewHalfHeight)
{
	_NewHalfHeight = FMath::Clamp(_NewHalfHeight, m_CrouchHalfHeight, m_StandHalfHeight);

	// 현재 Capsule Half Height
	const float CurrentHalfHeight = m_CapsuleComponent->GetUnscaledCapsuleHalfHeight();

	// 이번 프레임의 높이 변화량
	const float HeightDifference = _NewHalfHeight - CurrentHalfHeight;

	// 변화량이 거의 0이면 이동할 필요 없음
	if (FMath::IsNearlyZero(HeightDifference))
	{
		return;
	}

	
	// 서버만 Root 이동
	if (m_Player->HasAuthority())
	{
		// Root 이동량
		const FVector MoveDelta = FVector(0.f, 0.f, HeightDifference);

		FHitResult Hit;

		// 캡슐의 루트 (위치) 이동
		m_CharacterMovementComponent->SafeMoveUpdatedComponent
		(
			MoveDelta,
			m_Player->GetActorQuat(),
			false,
			Hit
		);
	}
	
	// Capsule 크기 변경
	/// 지형에 끼지 않도록 이동 후에 크기 변경
	m_CapsuleComponent->SetCapsuleSize(
		m_StandRadius,
		_NewHalfHeight,
		false
	);

	// Mesh 위치 보정
	FVector NewMeshRelativeLocation = m_StandMeshRelativeLocation;

	NewMeshRelativeLocation.Z += (m_StandHalfHeight - _NewHalfHeight);

	m_MeshComponent->SetRelativeLocation
	(
		NewMeshRelativeLocation,
		false,
		nullptr,
		ETeleportType::None
	);

	/*

	// 실제 월드에서 이동해야하는 거리로 변환
	const float WorldHalfHeightDifference = HeightDifference * m_CapsuleComponent->GetShapeScale();

	// Root Capsule 이동하면 Mesh 위치도 같이 이동
	const FVector MeshWorldLocationBeforeMove = m_MeshComponent->GetComponentLocation();

	FHitResult MoveHit;

	// Capsule 높이 줄이면 변화량 만큼 Actor 위치를 올려줘야함
	// 웅크리기 : HalfHeightDifference < 0.f
	if (HeightDifference < 0.f)
	{
		// 웅크리기 
		// 1. Capsule 높이 줄이기
		// 2. Root Capsule 아래로 이동
		m_CapsuleComponent->SetCapsuleSize(m_StandRadius, _NewHalfHeight, false);

		m_CharacterMovementComponent->SafeMoveUpdatedComponent(
			FVector
			(
				0.f,
				0.f,
				WorldHalfHeightDifference
			),
			m_CapsuleComponent->GetComponentQuat(),
			false,
			MoveHit,
			ETeleportType::TeleportPhysics
		);
	}
	// Capsule 높이 늘리면 변화량 만큼 Actor 위치를 내려줘야함
	// 일어서기 : HalfHeightDifference > 0.f
	else
	{
		m_CharacterMovementComponent->SafeMoveUpdatedComponent(
			FVector
			(
				0.f,
				0.f,
				WorldHalfHeightDifference
			),
			m_CapsuleComponent->GetComponentQuat(),
			false,
			MoveHit,
			ETeleportType::TeleportPhysics
		);

		m_CapsuleComponent->SetCapsuleSize(m_StandRadius, _NewHalfHeight, false);
	}

	RefreshFloorInformation();

	m_MeshComponent->SetWorldLocation
	(
		MeshWorldLocationBeforeMove,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	*/

	// 변경된 충돌 상태 갱신
	m_CapsuleComponent->UpdateOverlaps();
}

void UC_PoseColliderHandlerComponent::RefreshFloorInformation()
{
	if (!m_CapsuleComponent || !m_CharacterMovementComponent)
	{
		return;
	}

	// 다음 CharacterMovement Tick에서도 바닥 검사를 강제로 다시 수행하게 합니다.
	m_CharacterMovementComponent->bForceNextFloorCheck = true;

	// Walking 상태가 아니라면 CurrentFloor를 갱신할 필요가 없습니다.
	if (!m_CharacterMovementComponent->IsMovingOnGround())
	{
		return;
	}

	// 변경된 Capsule 위치에서 바닥 재검사
	FFindFloorResult NewFloorResult;

	m_CharacterMovementComponent->FindFloor
	(
		m_CapsuleComponent
		->GetComponentLocation(),
		NewFloorResult,
		false,
		nullptr
	);

	// CharacterMovement가 사용하는 현재 바닥 정보 갱신
	m_CharacterMovementComponent->CurrentFloor = NewFloorResult;

	// 걸을 수 있는 바닥이면 해당 바닥을 Character의 MovementBase로 등록합니다.
	// 걸을 수 없는 바닥이면 SetBaseFromFloor()가 MovementBase를 제거합니다.
	m_CharacterMovementComponent->SetBaseFromFloor(NewFloorResult);

	if (NewFloorResult.IsWalkableFloor())
	{
		// Capsule과 바닥 사이의 거리를 CharacterMovement가 사용하는 적절한 값으로 맞춥니다.
		// 완전히 0cm로 붙이는 것이 아니라, 보행 안정성을 위한 아주 작은 간격을 유지할 수 있습니다.
		m_CharacterMovementComponent->AdjustFloorHeight();
	}

	// 다음 Movement Tick에서도 최종 위치를 다시 확인
	m_CharacterMovementComponent->bForceNextFloorCheck = true;
}

void UC_PoseColliderHandlerComponent::FinishTransition()
{
	const float TargetHalfHeight = m_bTargetCrouched ? m_CrouchHalfHeight : m_StandHalfHeight;
	
	// 마지막에 정확한 목표 높이를 적용
	ApplyCapsuleHalfHeight(TargetHalfHeight);
	
	m_bIsCrouched = m_bTargetCrouched;
	m_bIsTransitioning = false;

	// Stand 상태로 돌아왔다면 Mesh 상대 위치 정확히 복원
	if (m_MeshComponent)
	{
		FVector FinalMeshRelativeLocation = m_StandMeshRelativeLocation;

		if (m_bIsCrouched)
		{
			FinalMeshRelativeLocation.Z += (m_StandHalfHeight - m_CrouchHalfHeight);
		}

		m_MeshComponent->SetRelativeLocation(
			FinalMeshRelativeLocation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics // 물리 시뮬레이션을 텔레포트 방식으로 처리
		);

		m_Player->CacheInitialMeshOffset
		(
			FinalMeshRelativeLocation,
			m_MeshComponent->GetRelativeRotation()
		);

		// 서버에 있는 클라이언트 캐릭터들은 OnRep() 함수를 호출하지 않아서 여기서 보정
		//const bool bRemoteClinetOnListenServer = m_Player->HasAuthority() && !m_Player->IsLocallyControlled();
		//
		//if (bRemoteClinetOnListenServer)
		//{
		//	m_Player->CacheInitialMeshOffset
		//	(
		//		FinalMeshRelativeLocation,
		//		m_MeshComponent->GetRelativeRotation()
		//	);
		//}
	}

	// 전환이 끝났으므로 Tick 비활성화
	SetComponentTickEnabled(false);

	// Player 에 자세 전환 완료 알림
	OnPoseTransitionFinished.Broadcast(m_bIsCrouched);
}

