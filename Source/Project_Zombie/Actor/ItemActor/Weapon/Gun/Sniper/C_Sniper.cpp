// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Sniper.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AC_Sniper::AC_Sniper()
{
	m_FireMode = EFireMode::Single;
	m_SpreadAngle = 0.0f; // 스나이퍼는 탄 퍼짐 0
}

bool AC_Sniper::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (!Super::OnStartFire(_WeaponUser)) return false;
	if (!m_bCanFire) return false;

	if (!ExecuteFire())
	{
		m_bIsFiring = false;
		m_bCanFire  = true;
		return false;
	}
		
	m_bIsFiring = true;
	m_bCanFire  = false;


	if (GetWorld() && m_FireRate > 0.0f)
		GetWorld()->GetTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_Sniper::ResetFireCooldown, m_FireRate, false);
	else
		ResetFireCooldown();
	
	return true;
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

		UGameplayStatics::ApplyDamage(
			HitActor,
			m_Damage * 0.5f,
			InstigatorController,
			Cast<AActor>(ShootingPawn ? (AActor*)ShootingPawn : (AActor*)this),
			nullptr);
	}

	Multicast_PlayAIFireEffects(ActualImpactPoint);
}