#include "C_ShotGun.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"



#include "TimerManager.h"
#include "Animation/AnimMontage.h"
#include "Serialization/AsyncPackageLoader.h"
#include "Utility/C_Util.h"


// m_SingleShellInsertTime -> 직접적인 SingleShell 넣는 AN에서 직접 조작(만약 해당 사항 버프가 있다면, Animation 동작 자체를 빠르게 재생 처리할 것)
AC_ShotGun::AC_ShotGun()
	: m_PelletCount(13)
	// , m_SingleShellInsertTime(0.6f)
	, m_bCanFire(true)
{
	// GunBase의 멤버변수
	m_SpreadAngle = 6.0f;
	m_FireMode    = EFireMode::Single;
	m_FireRate    = 0.8f;
}

bool AC_ShotGun::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	// Super에서 어차피 처리가 되는 중
	// if (!_WeaponUser) return false;

	m_OwnerPlayer = _WeaponUser;
	if (!m_OwnerPlayer) return false;
	
	// 재장전 하는 중 -> 재장전 모션 끝내기(예외적으로 여기서만 사격이 재장전의 우선순위를 이기는 중)
	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(m_PlayerReloadAnimation))
	{
		m_bIsReloading = false;
		// m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation); // 직접적으로 재장전 모션을 끊어줌 (이거 근데 어차피 Priority가 같아서 안해도 무방)
	}

	if (!Super::OnStartFire(_WeaponUser)) return false;
	if (!m_bCanFire) return false;

	m_bIsFiring = true;
	m_bCanFire  = false;

	if (!ExecuteFire()) return false;

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_ShotGun::ResetFireCooldown, m_FireRate, false);
	
	return true;
}

void AC_ShotGun::AN_OnSingleReloadEnd()
{
	// SingleShell insert 처리 시 호출되는 함수
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("[AC_ShotGun::AN_OnSingleReloadEnd] : OwnerPlayer nullptr",FColor::Red, 10.f);
		return;
	}
	
	if (!m_OwnerPlayer->IsLocallyControlled()) return;

	// 재장전이 끊긴 상황 (여기서도 예외처리를 한 번 해주긴 해야 함)
	if (!m_bIsReloading) return;

	// 장탄수 늘리기 및 UI 업데이트
	m_CurrentAmmo = FMath::Min(m_CurrentAmmo + 1, m_MaxAmmo);
	UpdateAmmoUI();

	// 재장전 함수 내에서 이 flag값 초기화되어 있지 않으면 씹어버림 && 실제로 재장전 처리가 끝난 경우이기 때문에 Reload flag false
	m_bIsReloading = false;
	
	// 이미 가득 찼다면 종료 (재장전 도중 취소는 여기서 따지지 않아도 됨 -> 타이밍이 장전 동작을 모두 수행한 경우의 타이밍임)
	if (m_CurrentAmmo >= m_MaxAmmo) return;

	// 다시금 똑같은 재장전 처리
	Reload(m_OwnerPlayer);
}

void AC_ShotGun::Multicast_StopReloadAnimation_Implementation()
{
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);
}

bool AC_ShotGun::ExecuteFire()
{
	if (!m_OwnerPlayer || !GetWorld()) return false;
	
	if (m_CurrentAmmo <= 0 || m_bIsReloading)
	{
		ReleaseTrigger();
		return false;
	}

	// AnimMontage에 의해 사격 모션이 끊기거나 제대로 재생처리가 이루어지지 않은 상황
	// Priority가 더 높은 동작이 수행되고 있다고 판단
	if (!PlayFireEffects())
	{
		ReleaseTrigger();
		return false;
	}

	m_CurrentAmmo--;
	if (m_CurrentAmmo <= 0)
	{
		m_CurrentAmmo = 0;
		m_bIsFiring = false;
	}
	
	UpdateAmmoUI();
	SpawnShellEject();

	FVector CameraLoc;
	FRotator CameraRot;
	if (APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController()))
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);
	else
		m_OwnerPlayer->GetActorEyesViewPoint(CameraLoc, CameraRot);

	TArray<FVector_NetQuantize> ImpactPoints{};
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
	return true;
}

void AC_ShotGun::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_ShotGun::Server_ShotgunFireEffects_Implementation(const TArray<FVector_NetQuantize>& ImpactPoints)
{
	Multicast_PlayShotgunFireEffects(ImpactPoints);
}

void AC_ShotGun::Multicast_PlayShotgunFireEffects_Implementation(const TArray<FVector_NetQuantize>& ImpactPoints)
{
	if (!m_WeaponMesh || !GetWorld()) return;

	// 사격 Animation (Gun이든, OwnerPlayer 든) 재생 처리
	PlayFireEffects();
	
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
	Super::OnSheathStart();
}

void AC_ShotGun::AIFire(const FVector& TargetLocation)
{
	if (!HasAuthority() || !m_WeaponMesh) return;

	m_CurrentAmmo = FMath::Max(0, m_CurrentAmmo - 1);

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector BaseDir = (TargetLocation - MuzzleStart).GetSafeNormal();

	TArray<FVector_NetQuantize> ImpactPoints;
	ImpactPoints.Reserve(m_PelletCount);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (m_OwnerEnemy) QueryParams.AddIgnoredActor(m_OwnerEnemy);

	if (AActor* AttachParent = GetAttachParentActor())
	{
		QueryParams.AddIgnoredActor(AttachParent);
	}

	for (int32 i = 0; i < m_PelletCount; ++i)
	{
		FVector PelletDir = FMath::VRandCone(BaseDir, FMath::DegreesToRadians(m_SpreadAngle));
		FVector EndLoc = MuzzleStart + (PelletDir * 10000.0f);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleStart, EndLoc, ECC_Visibility, QueryParams);
		FVector ImpactPoint = bHit ? HitResult.ImpactPoint : EndLoc;

		ImpactPoints.Add(ImpactPoint);

		if (bHit && HitResult.GetActor())
		{
			AActor* HitActor = HitResult.GetActor();

			APawn* ShootingPawn = Cast<APawn>(GetAttachParentActor());

			AController* InstigatorController = ShootingPawn ? ShootingPawn->GetController() : nullptr;
			float PelletDamage = m_Damage / static_cast<float>(m_PelletCount);
			UGameplayStatics::ApplyDamage(HitResult.GetActor(), PelletDamage * 0.5f, InstigatorController, this, nullptr);
		}
	}

	Multicast_PlayShotgunFireEffects(ImpactPoints);
}