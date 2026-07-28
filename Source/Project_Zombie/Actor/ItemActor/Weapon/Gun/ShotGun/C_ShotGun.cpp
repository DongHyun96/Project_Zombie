// Fill out your copyright notice in the Description page of Project Settings.

#include "C_ShotGun.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AC_ShotGun::AC_ShotGun()
{
	m_PelletCount = 8;
	m_SpreadAngle = 6.0f;
	m_FireMode = EFireMode::Single;
}

bool AC_ShotGun::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (m_bIsReloading)
	{
		EndReload();
	}
	return Super::OnStartFire(_WeaponUser);
}

void AC_ShotGun::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	PlayFireEffects_Local();
	Server_PullTrigger();

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_ShotGun::ResetFireCooldown, m_FireRate, false);
}

void AC_ShotGun::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_ShotGun::Server_ExecuteFire()
{
	ProcessShotgunPellets(m_Damage);
}

void AC_ShotGun::ProcessShotgunPellets(float BaseDamagePerPellet)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer) return;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float TraceRange = 3500.0f;
	FVector CameraEnd = CameraStart + (CameraForward * TraceRange);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(CameraHitResult, CameraStart, CameraEnd, ECC_Visibility, QueryParams);
	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector MainDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	// 8발 펠릿의 충돌 지점 수집용 배열
	TArray<FVector_NetQuantize> ImpactPoints;
	ImpactPoints.Reserve(m_PelletCount);

	// 8발 펠릿 사격 판정 Loop (서버)
	for (int32 i = 0; i < m_PelletCount; ++i)
	{
		FVector SpreadDir = FMath::VRandCone(MainDirection, FMath::DegreesToRadians(m_SpreadAngle));
		FVector MaxEndLocation = MuzzleStart + (SpreadDir * TraceRange);

		FHitResult PelletHitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(PelletHitResult, MuzzleStart, MaxEndLocation, ECC_Visibility, QueryParams);

		FVector ActualImpactPoint = bHit ? PelletHitResult.ImpactPoint : MaxEndLocation;
		ImpactPoints.Add(ActualImpactPoint);

		// 데미지 처리
		if (bHit)
		{
			if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(PelletHitResult.GetActor()))
			{
				UGameplayStatics::ApplyDamage(Enemy, BaseDamagePerPellet, m_OwnerPlayer->GetController(), this, nullptr);
			}
		}
	}

	// 서버에서 8개 충돌 지점을 한 번에 클라이언트들에게 멀티캐스트
	Multicast_PlayShotgunFireEffects(ImpactPoints);
}

void AC_ShotGun::Multicast_PlayShotgunFireEffects_Implementation(const TArray<FVector_NetQuantize>& ImpactPoints)
{
	// 1. 발사 애니메이션 재생 (1회)
	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}
	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	// 2. 탄피 배출 (딱 1회!)
	SpawnShellEject();

	// 3. 8발 펠릿 각각에 대해 GunBase와 동일한 Tracer & Impact 연출 적용
	if (!m_WeaponMesh || !GetWorld()) return;

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));

	for (const FVector_NetQuantize& ImpactPoint : ImpactPoints)
	{
		FVector ExplicitImpactPoint = FVector(ImpactPoint);
		FVector ShootDir = (ExplicitImpactPoint - MuzzleStart).GetSafeNormal();
		FRotator MuzzleRotation = ShootDir.Rotation();

		if (m_TracerFX)
		{
			UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				m_TracerFX,
				MuzzleStart,
				MuzzleRotation,
				FVector(1.0f),
				false
			);

			if (TracerComp)
			{
				float Distance = FVector::Distance(MuzzleStart, ExplicitImpactPoint);
				float Speed = 20000.0f; // 궤적 이동 속도
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

							// 탄착 지점 임팩트 FX 재생
							if (m_ImpactFX && GetWorld())
							{
								FRotator ImpactRotation = (-ShootDir).Rotation();
								UGameplayStatics::SpawnEmitterAtLocation(
									GetWorld(),
									m_ImpactFX,
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
}

void AC_ShotGun::Server_ExecuteReload()
{
	GetWorldTimerManager().SetTimer(m_ReloadLoopTimer, this, &AC_ShotGun::InsertSingleShell, m_SingleShellInsertTime, true);
}

void AC_ShotGun::InsertSingleShell()
{
	if (m_CurrentAmmo < m_MaxAmmo)
	{
		m_CurrentAmmo++;
		Multicast_PlayReloadEffects();
	}

	if (m_CurrentAmmo >= m_MaxAmmo)
	{
		EndReload();
	}
}

void AC_ShotGun::EndReload()
{
	m_bIsReloading = false;
	GetWorldTimerManager().ClearTimer(m_ReloadLoopTimer);
	Multicast_StopReloadAnimation();
}

void AC_ShotGun::Multicast_StopReloadAnimation_Implementation()
{
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);
	}
}