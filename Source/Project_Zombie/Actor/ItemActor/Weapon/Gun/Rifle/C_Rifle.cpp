// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Rifle.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Kismet/GameplayStatics.h"

AC_Rifle::AC_Rifle()
{
	m_FireMode = EFireMode::FullAuto;
}

void AC_Rifle::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	HandleAutomaticFire();

	// 연사 타이머 작동
	GetWorldTimerManager().SetTimer(m_AutoFireTimer, this, &AC_Rifle::HandleAutomaticFire, m_FireRate, true);
}

void AC_Rifle::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	GetWorldTimerManager().ClearTimer(m_AutoFireTimer);
}

void AC_Rifle::HandleAutomaticFire()
{
	if (m_CurrentAmmo <= 0 || !m_bIsFiring || m_bIsReloading)
	{
		ReleaseTrigger();
		return;
	}

	PlayFireEffects_Local();
	Server_PullTrigger();
}

void AC_Rifle::Server_ExecuteFire()
{
	ProcessSingleRifleShot(m_Damage);
}

void AC_Rifle::ProcessSingleRifleShot(float DamageVal)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer) return;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float TraceRange = 5000.0f;
	FVector CameraEnd = CameraStart + (CameraForward * TraceRange);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(CameraHitResult, CameraStart, CameraEnd, ECC_Visibility, QueryParams);
	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	if (m_SpreadAngle > 0.0f)
	{
		ShootDirection = FMath::VRandCone(ShootDirection, FMath::DegreesToRadians(m_SpreadAngle));
	}

	FVector FinalMuzzleEnd = MuzzleStart + (ShootDirection * TraceRange);
	FHitResult MuzzleHitResult;

	bool bHit = GetWorld()->LineTraceSingleByChannel(MuzzleHitResult, MuzzleStart, FinalMuzzleEnd, ECC_Visibility, QueryParams);

	// 1. 탄착 지점 결정 (맞았으면 ImpactPoint, 안 맞았으면 최종 도달 지점)
	FVector ImpactPoint = bHit ? MuzzleHitResult.ImpactPoint : FinalMuzzleEnd;

	// 2. 이펙트 멀티캐스트 호출 (궤적 LERP 이동 + 탄피 나이아가라 배출 + Impact 이펙트)
	Multicast_PlayFireEffects(ImpactPoint);

	// 3. 데미지 처리
	if (bHit)
	{
		if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(MuzzleHitResult.GetActor()))
		{
			UGameplayStatics::ApplyDamage(Enemy, DamageVal, m_OwnerPlayer->GetController(), this, nullptr);
		}
	}
}