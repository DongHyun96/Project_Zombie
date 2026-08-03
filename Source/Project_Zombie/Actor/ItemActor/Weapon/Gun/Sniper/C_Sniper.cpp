// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Sniper.h"
#include "Actor/Character/Player/C_BasicPlayer.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AC_Sniper::AC_Sniper()
{
	m_FireMode = EFireMode::Single;
	m_SpreadAngle = 0.0f; // 스나이퍼는 탄 퍼짐 0
}

void AC_Sniper::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	Client_ExecuteFire();

	if (GetWorld() && m_FireRate > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_Sniper::ResetFireCooldown, m_FireRate, false);
	}
	else
	{
		ResetFireCooldown();
	}
}

void AC_Sniper::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_Sniper::AIFire(const FVector& TargetLocation)
{
	if (!HasAuthority() || !m_WeaponMesh || m_CurrentAmmo <= 0) return;

	m_CurrentAmmo = FMath::Max(0, m_CurrentAmmo - 1);

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ShootDir = (TargetLocation - MuzzleStart).GetSafeNormal();
	FVector EndLocation = MuzzleStart + (ShootDir * 10000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (GetOwner()) QueryParams.AddIgnoredActor(GetOwner());

	// ★ AI 캐릭터(CopZombie) Ignore 처리
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
		if (!ShootingPawn && GetOwner()) ShootingPawn = Cast<APawn>(GetOwner());

		AController* InstigatorController = ShootingPawn ? ShootingPawn->GetController() : nullptr;

		UGameplayStatics::ApplyDamage(
			HitActor,
			m_Damage,
			InstigatorController,
			Cast<AActor>(ShootingPawn ? (AActor*)ShootingPawn : (AActor*)this),
			nullptr);
	}

	Multicast_PlayAIFireEffects(ActualImpactPoint);
}