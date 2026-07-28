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
	if (m_bInBurstCooldown || m_bIsFiring || m_bIsReloading || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;

	switch (m_FireMode)
	{
	case EFireMode::FullAuto:
		HandleAutomaticFire();
		GetWorldTimerManager().SetTimer(m_AutoFireTimer, this, &AC_Rifle::HandleAutomaticFire, m_FireRate, true);
		break;

	case EFireMode::Burst:
		m_BurstCount = 0;
		HandleBurstFire();
		GetWorldTimerManager().SetTimer(m_AutoFireTimer, this, &AC_Rifle::HandleBurstFire, m_FireRate, true);
		break;

	case EFireMode::Single:
		HandleAutomaticFire();
		break;
	}
}

void AC_Rifle::HandleBurstFire()
{
	if (m_CurrentAmmo <= 0 || m_bIsReloading || m_BurstCount >= m_MaxBurstCount)
	{
		GetWorldTimerManager().ClearTimer(m_AutoFireTimer);
		m_bIsFiring = false;
		return;
	}

	PlayFireEffects_Local();
	Server_PullTrigger();
	m_BurstCount++;

	if (m_BurstCount >= m_MaxBurstCount)
	{
		GetWorldTimerManager().ClearTimer(m_AutoFireTimer);
		m_bIsFiring = false;

		m_bInBurstCooldown = true;
		GetWorldTimerManager().SetTimer(
			m_BurstCooldownTimer,
			this,
			&AC_Rifle::ResetBurstCooldown,
			m_BurstCooldown,
			false
		);
	}
}

void AC_Rifle::ResetBurstCooldown()
{
	m_bInBurstCooldown = false;
}

void AC_Rifle::ReleaseTrigger()
{
	if (m_FireMode == EFireMode::Burst && m_BurstCount < m_MaxBurstCount && m_CurrentAmmo > 0)
	{
		m_bIsFiring = false;
		return;
	}

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

void AC_Rifle::SwitchFireMode()
{
	if (m_bIsFiring)
	{
		ReleaseTrigger();
	}

	// 모드 변경 시 진행 중이던 쿨타임도 리셋
	m_bInBurstCooldown = false;
	GetWorldTimerManager().ClearTimer(m_BurstCooldownTimer);

	switch (m_FireMode)
	{
	case EFireMode::Single:
		m_FireMode = EFireMode::Burst;
		break;
	case EFireMode::Burst:
		m_FireMode = EFireMode::FullAuto;
		break;
	case EFireMode::FullAuto:
		m_FireMode = EFireMode::Single;
		break;
	default:
		m_FireMode = EFireMode::Single;
		break;
	}

	Super::SwitchFireMode();
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

	FVector ImpactPoint = bHit ? MuzzleHitResult.ImpactPoint : FinalMuzzleEnd;

	Multicast_PlayFireEffects(ImpactPoint);

	if (bHit)
	{
		if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(MuzzleHitResult.GetActor()))
		{
			UGameplayStatics::ApplyDamage(Enemy, DamageVal, m_OwnerPlayer->GetController(), this, nullptr);
		}
	}
}