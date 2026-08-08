// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/C_FeetComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Sound/FootStep/C_PhysicalMaterial_FootSound.h"

#include "Kismet/GameplayStatics.h"


UC_FeetComponent::UC_FeetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	m_LeftFootSocketName = FName(TEXT("left_foot"));
	m_RightFootSocketName = FName(TEXT("right_foot"));

	m_LocalVolume = 1.f;
	m_RemoteVolume = 0.6f;

	m_CrouchPitch = 0.4f;

	m_FootstepAttenuation = nullptr;
	m_FootstepConcurrency = nullptr;
}


void UC_FeetComponent::BeginPlay()
{
	Super::BeginPlay();

	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
		return;
}

void UC_FeetComponent::PlayFootstep(bool _IsLeftFoot)
{
	if (!m_OwnerPlayer)
		return;

	if (m_OwnerPlayer->IsFalling())
		return;

	// 정지 블렌딩 중 발소리 방지
	if (m_OwnerPlayer->GetVelocity().SizeSquared2D() <= KINDA_SMALL_NUMBER)
		return;

	USkeletalMeshComponent* Mesh = m_OwnerPlayer->GetMesh();
	if (!Mesh)
		return;

	// 소켓에서 땅바닥으로 향하틑 시작점 끝점 설정
	const FName FootSocketName = (_IsLeftFoot) ? m_LeftFootSocketName : m_RightFootSocketName;
	const FVector FootLocation = Mesh->GetSocketLocation(FootSocketName);

	const FVector TraceStart = FootLocation + FVector(0.f, 0.f, 20.f);
	const FVector TraceEnd = FootLocation - FVector(0.f, 0.f, 100.f);

	// 라인 트레이스 검사
	FCollisionQueryParams Params = {};
	Params.AddIgnoredActor(m_OwnerPlayer);
	
	// 충돌이 검출되면, 대상의 물리재질 정보를 가져올것
	Params.bReturnPhysicalMaterial = true;

	FHitResult HitResult;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Footstep] Actor=%s / Component=%s / PhysMaterial=%s / FaceIndex=%d"),
		*GetNameSafe(HitResult.GetActor()),
		*GetNameSafe(HitResult.GetComponent()),
		*GetNameSafe(HitResult.PhysMaterial.Get()),
		HitResult.FaceIndex
	);

	if (!bHit)
		return;

	UC_PhysicalMaterial_FootSound* FootMaterial = Cast<UC_PhysicalMaterial_FootSound>(HitResult.PhysMaterial.Get());

	if (!FootMaterial)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[Footstep] FootMaterial Cast 실패 / 실제 PhysMaterial=%s"),
			*GetNameSafe(HitResult.PhysMaterial.Get())
		);

		return;
	}

	// 플레이어 이동 자세와 발 위치에 따라 맞는 사운드 재생
	USoundBase* FootstepSound = nullptr;
	float PitchMultiplier = 1.f;

	switch (m_OwnerPlayer->GetPlayerMoveState())
	{
	case EPlayerPoseState::Walk:
		FootstepSound = FootMaterial->GetWalkSound(_IsLeftFoot);
		break;

	case EPlayerPoseState::Sprint:
		FootstepSound = FootMaterial->GetSprintSound(_IsLeftFoot);
		break;

	case EPlayerPoseState::Crouch:
		FootstepSound = FootMaterial->GetWalkSound(_IsLeftFoot);
		PitchMultiplier = m_CrouchPitch;
		break;

	default:
		return;
	}

	if (!FootstepSound)
		return;


	// 로컬 플레이어일 경우, 로컬 볼륨을 적용하고, 원격 플레이어일 경우 원격 볼륨을 적용
	const float VolumeMultiplier = m_OwnerPlayer->IsLocallyControlled() ? m_LocalVolume : m_RemoteVolume;

	UGameplayStatics::PlaySoundAtLocation(
		this,
		FootstepSound,
		HitResult.ImpactPoint,
		VolumeMultiplier,
		PitchMultiplier,
		0.f,
		m_FootstepAttenuation,
		m_FootstepConcurrency
	);
}

void UC_FeetComponent::PlayLandingSound(const FHitResult& _Hit)
{
	if (!m_OwnerPlayer)
		return;

	USkeletalMeshComponent* Mesh = m_OwnerPlayer->GetMesh();
	if (!Mesh)
		return;

	const FVector TraceStart = _Hit.ImpactPoint + FVector(0.f, 0.f, 20.f);
	const FVector TraceEnd = _Hit.ImpactPoint - FVector(0.f, 0.f, 100.f);

	// 라인 트레이스 검사
	FCollisionQueryParams Params = {};
	Params.AddIgnoredActor(m_OwnerPlayer);

	// 충돌이 검출되면, 대상의 물리재질 정보를 가져올것
	Params.bReturnPhysicalMaterial = true;

	FHitResult HitResult;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	if (!bHit)
		return;

	UC_PhysicalMaterial_FootSound* FootMaterial = Cast<UC_PhysicalMaterial_FootSound>(HitResult.PhysMaterial.Get());
	if (!FootMaterial)
		return;

	if (!FootMaterial->GetLandingSound())
		return;

	UGameplayStatics::PlaySoundAtLocation(
		this,
		FootMaterial->GetLandingSound(),
		HitResult.ImpactPoint,
		m_LocalVolume
	);
}


