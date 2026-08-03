// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AIGunUsageComponent.h"

#include "Actor/Character/C_BasicCharacter.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Particles/ParticleSystemComponent.h"
#include "Actor/Components/C_PingSystemComponent.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"


UC_AIGunUsageComponent::UC_AIGunUsageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UC_AIGunUsageComponent::BeginPlay()
{
	Super::BeginPlay();
	
	m_OwnerGun = Cast<AC_GunBase>(GetOwner());
	if (!m_OwnerGun) UC_Util::Print("From UC_AIGunUsageComponent::BeginPlay : This Component is for GunType Actor", FColor::Red, 10.f);
}

bool UC_AIGunUsageComponent::AIFire()
{
	if (!m_OwnerGun || !m_OwnerGun->HasAuthority()) return false;
	if (m_OwnerGun->GetCurrentAmmo() <= 0) return false;

	AActor* Target = m_WeaponCopZombieUser && m_WeaponCopZombieUser->GetZombieController() ?
		m_WeaponCopZombieUser->GetZombieController()->GetCurrentBBTarget() : nullptr;

	USkeletalMeshComponent* WeaponMesh = m_OwnerGun->GetWeaponMesh();
	FVector StartLocation = WeaponMesh ? WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash")) : m_OwnerGun->GetActorLocation();
	FVector TargetLocation = FVector::ZeroVector;

	if (Target)
	{
		FVector TargetCenter = Target->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		float SpreadAmount = 0.0f;
		FVector RandomOffset = FVector(
			FMath::RandRange(-SpreadAmount, SpreadAmount),
			FMath::RandRange(-SpreadAmount, SpreadAmount),
			FMath::RandRange(-SpreadAmount, SpreadAmount)
		);
		TargetLocation = TargetCenter + RandomOffset;
	}
	else
	{
		TargetLocation = StartLocation + (m_WeaponCopZombieUser ? m_WeaponCopZombieUser->GetActorForwardVector() : m_OwnerGun->GetActorForwardVector()) * 5000.0f;
	}

	m_OwnerGun->AIFire(TargetLocation);

	return true;
}

bool UC_AIGunUsageComponent::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	m_WeaponCopZombieUser = Cast<AC_CopZombie>(_ParentMesh->GetOwner());
	if (!m_WeaponCopZombieUser)
	{
		UC_Util::Print("From UC_AIGunUsageComponent::AttachToHand : Only Cop Owner among enemy can own GunBase Weapon!", FColor::Red, 10.f);
		return false;
	}

	m_OwnerGun->m_OwnerPlayer = nullptr;

	m_OwnerGun->SetOwner(m_WeaponCopZombieUser);

	const bool Attached = m_OwnerGun->AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_OwnerGun->s_HandSocketName
	);

	if (!Attached)
	{
		UC_Util::Print("From UC_AIGunUsageComponent::AttachToHand : AttachToComponent failed!", FColor::Red, 10.f);
		return false;
	}

	// MaxAmmoCount로 탄창 초기화 처리
	m_OwnerGun->m_CurrentAmmo = m_OwnerGun->m_MaxAmmo;

	// 이미 사격중이었던 Weapon인 경우, Trigger 해제
	m_OwnerGun->ReleaseTrigger();

	return true;
}

bool UC_AIGunUsageComponent::DetachFromHand()
{
	// 이 무기를 사용중인 CopZombie가 없을 때(또는 Valid하지 않은 경우)
	if (!m_WeaponCopZombieUser) return false;
	
	m_OwnerGun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// RootComponent(MainCollider) 비활성화
	m_OwnerGun->m_Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// WeaponMesh의 Physics 활성화 처리 이전에, 이전 주인과의 충돌을 비활성화 처리
	m_OwnerGun->GetWeaponMesh()->IgnoreActorWhenMoving(m_WeaponCopZombieUser, true);
	
	// 지형지물 충돌 활성화 및 Physics 활성화 처리
	m_OwnerGun->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_OwnerGun->GetWeaponMesh()->SetSimulatePhysics(true);

	// 캐릭터가 바라보는 방향으로 살짝 위로 Drop 되는 느낌 처리를 줌
	
	const FVector ImpulseVector = m_WeaponCopZombieUser->GetActorForwardVector() * 350.f +
								  m_WeaponCopZombieUser->GetActorUpVector() * 200.f;
	
	m_OwnerGun->GetWeaponMesh()->AddImpulse(ImpulseVector, NAME_None, true);

	GetWorld()->GetTimerManager().SetTimer
	(
		m_GunMeshStoppedCheckTimer,
		this,
		&UC_AIGunUsageComponent::HandleGunMeshPhysicsStopped,
		0.5f,
		true
	);

	// OwnerCopZombie 초기화
	m_WeaponCopZombieUser = nullptr;
	
	return true;
}

FVector UC_AIGunUsageComponent::AIProcessLineTraceDamage(float _DamageVal)
{
	if (!m_WeaponCopZombieUser || !m_OwnerGun) return FVector::ZeroVector;

	USkeletalMeshComponent* WeaponMesh = m_OwnerGun->GetWeaponMesh();
	if (!WeaponMesh) return FVector::ZeroVector;

	const FVector StartLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));

	// 1. AIController로부터 현재 블랙보드 타겟 가져오기
	AActor* Target = m_WeaponCopZombieUser->GetZombieController() ?
		m_WeaponCopZombieUser->GetZombieController()->GetCurrentBBTarget() : nullptr;

	FVector EndLocation = FVector::ZeroVector;

	if (Target)
	{
		// 2. Target의 발바닥(Origin) 대신 가슴/상체 위치 보정 (+Z 50.f)
		FVector TargetCenter = Target->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

		// 3. (선택사항) AI 사격 탄퍼짐/오차 추가 (완벽한 명중을 원하시면 Offset을 FVector::ZeroVector로 고정)
		float SpreadAmount = 15.0f; // 오차 범위 (센티미터 단위)
		FVector RandomOffset = FVector(
			FMath::RandRange(-SpreadAmount, SpreadAmount),
			FMath::RandRange(-SpreadAmount, SpreadAmount),
			FMath::RandRange(-SpreadAmount, SpreadAmount)
		);

		FVector AimDir = (TargetCenter - StartLocation).GetSafeNormal();

		// 총구에서 타겟 상체 방향으로 사격 거리(예: 5000 units)만큼 뻗어나감
		EndLocation = StartLocation + (AimDir + RandomOffset * 0.001f).GetSafeNormal() * 5000.0f;
	}
	else
	{
		// 타겟이 없는 경우 좀비가 바라보는 정면 방향으로 발사
		FVector ForwardDir = m_WeaponCopZombieUser->GetActorForwardVector();
		EndLocation = StartLocation + ForwardDir * 5000.0f;
	}

	// 4. LineTrace 수행
	FHitResult HitResult{};
	FCollisionQueryParams QueryParams{};
	QueryParams.AddIgnoredActor(m_OwnerGun);
	QueryParams.AddIgnoredActor(m_WeaponCopZombieUser);

	bool bHasHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	const FVector ActualImpactPoint = bHasHit ? HitResult.ImpactPoint : EndLocation;

	// 5. 서버에서 데미지 전달
	if (bHasHit)
	{
		if (AC_BasicCharacter* HitCharacter = Cast<AC_BasicCharacter>(HitResult.GetActor()))
		{
			UGameplayStatics::ApplyDamage(
				HitCharacter,
				_DamageVal,
				m_WeaponCopZombieUser->GetController(),
				m_OwnerGun,
				nullptr
			);
		}
	}

	return ActualImpactPoint;
}

void UC_AIGunUsageComponent::HandleGunMeshPhysicsStopped()
{
	// 아직 SimulatePhysics 처리에 의해 움직이는 중
	if (m_OwnerGun->GetWeaponMesh()->IsAnyRigidBodyAwake()) return;
	
	/* 움직임이 멈춤 */
	
	GetWorld()->GetTimerManager().ClearTimer(m_GunMeshStoppedCheckTimer);
	
	m_OwnerGun->GetWeaponMesh()->IgnoreActorWhenMoving(m_WeaponCopZombieUser, false);
	m_OwnerGun->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_OwnerGun->GetWeaponMesh()->SetSimulatePhysics(false);

	// OwnerGun Actor의 위치 정상복구 처리
	const FTransform& MeshWorldTransform = m_OwnerGun->GetWeaponMesh()->GetComponentTransform();
	m_OwnerGun->SetActorTransform(MeshWorldTransform);
	m_OwnerGun->GetWeaponMesh()->SetRelativeTransform(FTransform::Identity);
	
	// 파밍 처리 가능하게끔 MainCollider 활성화 (Overlap 검사)
	m_OwnerGun->m_Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 무기 외곽선 활성화
	m_OwnerGun->GetWeaponMesh()->SetCustomDepthStencilValue(1);

	// 무기 WorldPingActor 스폰 (이전 Player 주인의 PingSystemComponent 사용) -> 원칙 : 핑 정보는 인당 하나만 표시로 무조건 통일
	if (!m_PrevOwnerPlayer)
	{
		UC_Util::Print("From UC_AIGunUsageComponent::HandleGunMeshPhysicsStopped : Prev OwnerPlayer nullptr!", FColor::Red, 10.f);
		return;
	}

	// TODO : 이거 Server 쪽으로 request 처리하여 일괄 스폰처리되게 수정할 것
	
	m_PrevOwnerPlayer->GetPingSystemComponent()->SpawnFullPing
	(
		m_OwnerGun->GetActorLocation(),
		EGamePingType::GunBaseMarker,
		m_OwnerGun // Last 유발자를 이 Gun으로 처리 -> 추후 떨군 무기를 먹었을 때 해당 핑이 아직 활성화 중인 경우 HidePing 처리를 해야하는지 확인할 때 사용
	);
	
	m_PrevOwnerPlayer = nullptr;
}

void UC_AIGunUsageComponent::Multicast_PlayAIFireEffects_Implementation(FVector_NetQuantize ImpactPoint)
{
	if (IsRunningDedicatedServer() || !m_OwnerGun) return;

	USkeletalMeshComponent* WeaponMesh = m_OwnerGun->GetWeaponMesh();
	if (!WeaponMesh) return;

	// 3. 탄피 배출
	m_OwnerGun->SpawnShellEject();

	FVector MuzzleStart = WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ExplicitImpactPoint = FVector(ImpactPoint);
	FVector ShootDir = (ExplicitImpactPoint - MuzzleStart).GetSafeNormal();
	FRotator MuzzleRotation = ShootDir.Rotation();

	if (UParticleSystem* TracerFX = m_OwnerGun->GetTracerFX())
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			TracerFX,
			MuzzleStart,
			MuzzleRotation,
			FVector(1.0f),
			false
		);

		if (TracerComp)
		{
			float Distance = FVector::Distance(MuzzleStart, ExplicitImpactPoint);
			float Speed = 20000.0f;
			float FlyTime = Distance / Speed;

			TSharedPtr<float> ElapsedTime = MakeShared<float>(0.0f);
			TSharedPtr<FTimerHandle> TracerTimerHandle = MakeShared<FTimerHandle>();

			GetWorld()->GetTimerManager().SetTimer(
				*TracerTimerHandle,
				[TracerComp, MuzzleStart, ExplicitImpactPoint, ShootDir, FlyTime, ElapsedTime, TracerTimerHandle, this]() mutable
				{
					if (!TracerComp || !TracerComp->IsValidLowLevel()) return;

					*ElapsedTime += 0.01f;
					float Alpha = FMath::Clamp(*ElapsedTime / FlyTime, 0.0f, 1.0f);

					FVector CurrentLoc = FMath::Lerp(MuzzleStart, ExplicitImpactPoint, Alpha);
					TracerComp->SetWorldLocation(CurrentLoc);

					if (Alpha >= 1.0f)
					{
						TracerComp->DeactivateSystem();
						TracerComp->DestroyComponent();

						// 탄착 지점에 Impact 이펙트 스폰
						if (m_OwnerGun && m_OwnerGun->GetImpactFX() && GetWorld())
						{
							FRotator ImpactRotation = (-ShootDir).Rotation();
							UGameplayStatics::SpawnEmitterAtLocation(
								GetWorld(),
								m_OwnerGun->GetImpactFX(),
								ExplicitImpactPoint,
								ImpactRotation,
								FVector(1.0f),
								true
							);
						}

						if (GetWorld() && TracerTimerHandle.IsValid())
						{
							GetWorld()->GetTimerManager().ClearTimer(*TracerTimerHandle);
						}
					}
				},
				0.01f,
				true
			);
		}
	}
}