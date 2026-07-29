#include "C_ShotGun.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
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

// ---------------------------------------------------------------------
// [재장전 처리]
// ---------------------------------------------------------------------
bool AC_ShotGun::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;
	m_OwnerPlayer = _WeaponUser;

	if (m_bIsFiring || m_bIsReloading || m_CurrentAmmo >= m_MaxAmmo) return false;

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
	if (m_bIsReloading) return;

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
	if (!HasAuthority() || !m_bIsReloading) return;

	if (m_CurrentAmmo < m_MaxAmmo)
	{
		m_CurrentAmmo++;

		// ★ [핵심] 서버가 클라이언트 PC에 "1발 들어갔으니 UI와 Ammo 변수 업데이트해!" 전달
		Client_OnSingleShellInserted(m_CurrentAmmo);

		if (m_CurrentAmmo >= m_MaxAmmo)
		{
			EndReload();
			return;
		}

		Multicast_PlayReloadEffects();
	}
}

// ★ 클라이언트 로컬 PC에서 실행되는 함수
void AC_ShotGun::Client_OnSingleShellInserted_Implementation(int32 NewAmmo)
{
	m_CurrentAmmo = NewAmmo;
	UpdateAmmoUI(); // 클라이언트 본인의 HUD UI 업데이트
}

void AC_ShotGun::EndReload()
{
	m_bIsReloading = false;

	GetWorldTimerManager().ClearTimer(m_ReloadLoopTimer);

	if (HasAuthority())
	{
		// ★ [핵심] 서버에서 장전 끝났으니 클라이언트의 m_bIsReloading 상태도 false로 해제
		Client_EndReload();
		Multicast_StopReloadAnimation();
	}
}

// ★ 클라이언트 로컬 PC에서 실행되는 장전 종료 함수
void AC_ShotGun::Client_EndReload_Implementation()
{
	m_bIsReloading = false;
	m_bIsFiring = false;
	UpdateAmmoUI();
}

void AC_ShotGun::Multicast_StopReloadAnimation_Implementation()
{
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);
	}
}

// ---------------------------------------------------------------------
// [사격 처리]
// ---------------------------------------------------------------------
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

	// 1. 탄약 차감 및 UI 갱신
	m_CurrentAmmo--;
	if (m_CurrentAmmo <= 0)
	{
		m_CurrentAmmo = 0;
		m_bIsFiring = false;
	}
	UpdateAmmoUI();

	// 2. 로컬 화면 탄피 배출 (클라이언트 본인 화면용 1발)
	PlayFireEffects_Client();
	SpawnShellEject();

	if (!m_OwnerPlayer || !GetWorld()) return;

	// 3. 카메라 트레이스 준비
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

	float PelletDamage = m_Damage / static_cast<float>(m_PelletCount);

	TArray<FVector_NetQuantize> ImpactPoints;
	ImpactPoints.Reserve(m_PelletCount);

	// 4. 8개 펠릿 라인트레이스 및 데미지 전달
	for (int32 i = 0; i < m_PelletCount; ++i)
	{
		AActor* HitActor = nullptr;
		FVector ImpactPoint = LineTraceDamage(CameraLoc, CameraRot, PelletDamage, m_SpreadAngle, HitActor);
		ImpactPoints.Add(ImpactPoint);

		if (HitActor && HitActor->IsA<AC_BasicEnemy>())
		{
			Server_ExecuteFire(ImpactPoint, HitActor);
		}
	}

	// 5. 서버 및 타인에게 "사격 연출 1회(탄피 1개 배출)"를 동기화하기 위해 1번만 RPC 호출
	Server_ShotgunFireEffects();

	// 6. 로컬 궤적 및 임팩트 이펙트
	Multicast_PlayShotgunFireEffects(ImpactPoints);
}

void AC_ShotGun::Server_ShotgunFireEffects_Implementation()
{
	Multicast_PlayFireEffects(FVector::ZeroVector);
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