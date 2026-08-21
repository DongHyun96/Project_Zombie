// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Rifle.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

AC_Rifle::AC_Rifle()
{
	m_FireMode = EFireMode::FullAuto;
	m_SpreadAngle = 3.5f;
}

void AC_Rifle::HandleAutomaticFire()
{
	if (m_CurrentAmmo <= 0 || !m_bIsFiring)
	{
		ReleaseTrigger();
		return;
	}

	ExecuteFire();

	if (m_FireMode == EFireMode::Single)
	{
		ReleaseTrigger();
	}
}

void AC_Rifle::HandleBurstFire()
{
	// 점사 사격을 끝내야하는 조건
	if (m_CurrentAmmo <= 0 || m_bIsReloading || m_BurstCount >= m_MaxBurstCount)
	{
		GetWorldTimerManager().ClearTimer(m_AutoFireTimer);
		m_bIsFiring = false;
		return;
	}

	if (!ExecuteFire()) // 모종의 이유로 ExecuteFire 자체가 끊긴상황
	{
		GetWorldTimerManager().ClearTimer(m_AutoFireTimer);
		m_bIsFiring = false;
		return;
	}
	
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

bool AC_Rifle::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (!Super::OnStartFire(_WeaponUser)) return false;
	
	if (m_bInBurstCooldown) return false;
	
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
	default:
	{
		UC_Util::Print("[AC_Rifle::OnStartFire] : Wrong FireMode received", FColor::Red, 10.f);
		return false;
	}
	}
	
	return true;
}

void AC_Rifle::ReleaseTrigger()
{
	// Burst 모드일 경우 예외처리 (가 왜 필요한지 잘 모르겠음)
	// 어차피 Super::ReleaseTrigger 자체도 m_bIsFiring = false만 처리를 하는 중이고, AutoFireTimer 해제 처리는
	// 어떤 FireMode이던지 상관없게 해주어도 무방할 거 같은데?
	// 위와 같은 이유로 아래와 같이 수정
	Super::ReleaseTrigger();
	GetWorldTimerManager().ClearTimer(m_AutoFireTimer);
}

void AC_Rifle::SwitchFireMode()
{
	if (m_bIsFiring) ReleaseTrigger();

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

	const FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	const FVector BaseDir     = (TargetLocation - MuzzleStart).GetSafeNormal();
	const FVector FinalDir    = FMath::VRandCone(BaseDir, FMath::DegreesToRadians(m_SpreadAngle));
	const FVector EndLocation = MuzzleStart + (FinalDir * 7000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	// 이거 둘 다 OwnerEnemy임
	if (m_OwnerEnemy) QueryParams.AddIgnoredActor(m_OwnerEnemy);
	if (AActor* AttachParent = GetAttachParentActor())
	{
		QueryParams.AddIgnoredActor(AttachParent);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleStart, EndLocation, ECC_Visibility, QueryParams);
	FVector ActualImpactPoint = bHit ? HitResult.ImpactPoint : EndLocation;

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();

		APawn* ShootingPawn = Cast<APawn>(GetAttachParentActor());
		AController* InstigatorController = ShootingPawn ? ShootingPawn->GetController() : nullptr;

		UGameplayStatics::ApplyDamage
		(
			HitActor,
			m_Damage * 0.5f,
			InstigatorController,
			this,
			nullptr
		);
	}

	Multicast_PlayAIFireEffects(ActualImpactPoint);
}