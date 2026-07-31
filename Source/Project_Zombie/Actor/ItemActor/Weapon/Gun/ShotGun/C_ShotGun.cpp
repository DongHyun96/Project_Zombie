#include "C_ShotGun.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#include "TimerManager.h"
#include "Animation/AnimMontage.h"


AC_ShotGun::AC_ShotGun()
{
	m_PelletCount = 8;
	m_SpreadAngle = 6.0f;
	m_FireMode = EFireMode::Single;
	m_FireRate = 0.8f;
	m_SingleShellInsertTime = 0.6f;
	m_bCanFire = true;
}

bool AC_ShotGun::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;

	if (m_bIsReloading)
	{
		EndReload();
	}

	return Super::OnStartFire(_WeaponUser);
}

bool AC_ShotGun::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;
	m_OwnerPlayer = _WeaponUser;

	// 이미 재장전 중이거나 탄약이 꽉 찼다면 거부
	if (m_bIsReloading || m_CurrentAmmo >= m_MaxAmmo) return false;

	m_bIsFiring = false;
	m_bIsReloading = true;

	if (!HasAuthority())
	{
		Server_StartReload();
		return true;
	}

	Multicast_PlayReloadEffects();

	GetWorldTimerManager().SetTimer(
		m_ReloadLoopTimer,
		this,
		&AC_ShotGun::InsertSingleShell,
		m_SingleShellInsertTime,
		true
	);

	return true;
}

void AC_ShotGun::Server_StartReload_Implementation()
{
	// 탄약이 이미 꽉 찬 경우 거부
	if (m_CurrentAmmo >= m_MaxAmmo) return;

	m_bIsFiring = false;
	m_bIsReloading = true;

	Multicast_PlayReloadEffects();

	GetWorldTimerManager().SetTimer(
		m_ReloadLoopTimer,
		this,
		&AC_ShotGun::InsertSingleShell,
		m_SingleShellInsertTime,
		true
	);
}

void AC_ShotGun::InsertSingleShell()
{
	if (!HasAuthority()) return;

	// 재장전 도중 취소되었거나 이미 가득 찼다면 종료
	if (!m_bIsReloading || m_CurrentAmmo >= m_MaxAmmo)
	{
		EndReload();
		return;
	}

	m_CurrentAmmo++;

	// 클라이언트 UI 및 탄약 동기화
	Client_OnSingleShellInserted(m_CurrentAmmo);

	if (m_CurrentAmmo >= m_MaxAmmo)
	{
		EndReload();
		return;
	}

	Multicast_PlayReloadEffects();
}

void AC_ShotGun::Client_OnSingleShellInserted_Implementation(int32 NewAmmo)
{
	m_CurrentAmmo = NewAmmo;
	m_bIsReloading = true; // 클라이언트 재장전 상태 유지
	UpdateAmmoUI();
}

void AC_ShotGun::EndReload()
{
	m_bIsReloading = false;

	GetWorldTimerManager().ClearTimer(m_ReloadLoopTimer);

	if (HasAuthority())
	{
		Client_EndReload();
		Multicast_StopReloadAnimation();
	}
}

void AC_ShotGun::Client_EndReload_Implementation()
{
	m_bIsReloading = false;
	m_bIsFiring = false;
	m_bCanFire = true;
	UpdateAmmoUI();
}

void AC_ShotGun::Multicast_StopReloadAnimation_Implementation()
{
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);
	}
}

void AC_ShotGun::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	Client_ExecuteFire();

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_ShotGun::ResetFireCooldown, m_FireRate, false);
}

void AC_ShotGun::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_ShotGun::Client_ExecuteFire()
{
	if (m_CurrentAmmo <= 0 || m_bIsReloading)
	{
		m_bIsFiring = false;
		return;
	}

	// ★ 핵심 수정: 서버(HasAuthority)에서는 아래 Server_ShotgunFireEffects에서 차감할 것이므로,
	// 클라이언트(순수 클라)일 때만 여기서 로컬 탄약을 차감하고 UI를 올립니다.
	if (!HasAuthority())
	{
		m_CurrentAmmo--;
		if (m_CurrentAmmo <= 0)
		{
			m_CurrentAmmo = 0;
			m_bIsFiring = false;
		}
		UpdateAmmoUI();
	}

	PlayFireEffects_Client();
	SpawnShellEject();

	if (!m_OwnerPlayer || !GetWorld()) return;

	FVector CameraLoc;
	FRotator CameraRot;
	if (APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController()))
	{
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);
	}
	else
	{
		m_OwnerPlayer->GetActorEyesViewPoint(CameraLoc, CameraRot);
	}

	TArray<FVector_NetQuantize> ImpactPoints;
	ImpactPoints.Reserve(m_PelletCount);

	for (int32 i = 0; i < m_PelletCount; ++i)
	{
		AActor* HitActor = nullptr;
		FVector ImpactPoint = LineTraceDamage(CameraLoc, CameraRot, HitActor);

		if (ImpactPoint.IsZero())
		{
			FVector ShootingDir = CameraRot.Vector();
			ImpactPoint = CameraLoc + (ShootingDir * 10000.0f);
		}

		ImpactPoints.Add(ImpactPoint);

		if (HitActor && HitActor->IsA<AC_BasicEnemy>())
		{
			Server_ExecuteFire(ImpactPoint, HitActor);
		}
	}

	// 서버로 이펙트 연출 및 서버 측 탄약 차감 전송
	Server_ShotgunFireEffects(ImpactPoints);
}

void AC_ShotGun::Server_ShotgunFireEffects_Implementation(const TArray<FVector_NetQuantize>& ImpactPoints)
{
	// 서버 메모리 탄약 차감 (서버 플레이어가 쐈든, 클라이언트가 RPC 보냈든 무조건 여기서 1발 차감)
	if (m_CurrentAmmo > 0)
	{
		m_CurrentAmmo--;
	}

	// ★ 서버 플레이어가 직접 쏜 경우에도 UI를 최신 탄약 수로 갱신해줍니다.
	if (HasAuthority() && m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
	{
		UpdateAmmoUI();
	}

	Multicast_PlayShotgunFireEffects(ImpactPoints);
}

void AC_ShotGun::Multicast_PlayShotgunFireEffects_Implementation(const TArray<FVector_NetQuantize>& ImpactPoints)
{
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
				float Speed = 20000.0f;
				float FlyTime = FMath::Max(0.01f, Distance / Speed);

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

void AC_ShotGun::OnSheathStart()
{
	GetWorldTimerManager().ClearTimer(m_ShotCooldownTimer);
	EndReload();

	Super::OnSheathStart();

}