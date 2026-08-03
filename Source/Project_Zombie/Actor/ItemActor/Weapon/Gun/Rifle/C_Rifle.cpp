// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Rifle.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Kismet/GameplayStatics.h"

AC_Rifle::AC_Rifle()
{
	m_FireMode = EFireMode::FullAuto;
	m_SpreadAngle = 3.5f;
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

void AC_Rifle::HandleAutomaticFire()
{
	if (m_CurrentAmmo <= 0 || !m_bIsFiring || m_bIsReloading)
	{
		ReleaseTrigger();
		return;
	}

	Client_ExecuteFire();

	if (m_FireMode == EFireMode::Single)
	{
		ReleaseTrigger();
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

	Client_ExecuteFire();
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

void AC_Rifle::AIFire(const FVector& TargetLocation)
{
	if (!HasAuthority() || !m_WeaponMesh || m_CurrentAmmo <= 0) return;

	m_CurrentAmmo = FMath::Max(0, m_CurrentAmmo - 1);

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector BaseDir = (TargetLocation - MuzzleStart).GetSafeNormal();

	FVector FinalDir = FMath::VRandCone(BaseDir, FMath::DegreesToRadians(m_SpreadAngle));
	FVector EndLocation = MuzzleStart + (FinalDir * 7000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);                     // 무기 자신 무시
	if (m_OwnerEnemy) QueryParams.AddIgnoredActor(m_OwnerEnemy); // 무기 Owner 무시

	// ★ 핵심 추가: 무기를 들고 있는 AI 캐릭터(CopZombie)도 Ignore 처리
	if (AActor* AttachParent = GetAttachParentActor())
	{
		QueryParams.AddIgnoredActor(AttachParent);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleStart, EndLocation, ECC_Visibility, QueryParams);
	FVector ActualImpactPoint = bHit ? HitResult.ImpactPoint : EndLocation;

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();

		// ★ 핵심 수정: HitActor가 플레이어 캐릭터인지 확인 후 대미지 전달
		APawn* ShootingPawn = Cast<APawn>(GetAttachParentActor());
		AController* InstigatorController = ShootingPawn ? ShootingPawn->GetController() : nullptr;

		UGameplayStatics::ApplyDamage
		(
			HitActor,
			m_Damage,
			InstigatorController,
			this,
			nullptr
		);
	}

	Multicast_PlayAIFireEffects(ActualImpactPoint);
}