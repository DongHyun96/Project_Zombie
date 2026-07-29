// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Sniper.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Kismet/GameplayStatics.h"

AC_Sniper::AC_Sniper()
{
	m_FireMode = EFireMode::Single;
}

void AC_Sniper::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	PlayFireEffects_Local();
	Server_PullTrigger();

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_Sniper::ResetFireCooldown, m_FireRate, false);
}

void AC_Sniper::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_Sniper::Server_ExecuteFire()
{
	ProcessSniperShot(m_Damage);
}

void AC_Sniper::ProcessSniperShot(float DamageVal)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer) return;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	// [1단계] 카메라 정중앙 스코프/크로스헤어 라인트레이스
	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float TraceRange = 10000.0f; // 100m
	FVector CameraEnd = CameraStart + (CameraForward * TraceRange);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(CameraHitResult, CameraStart, CameraEnd, ECC_Visibility, QueryParams);
	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	// [2단계] 총구에서 목표 지점으로 정밀 정렬
	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();
	FVector FinalMuzzleEnd = MuzzleStart + (ShootDirection * TraceRange);

	FHitResult MuzzleHitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(MuzzleHitResult, MuzzleStart, FinalMuzzleEnd, ECC_Visibility, QueryParams);

	// 1. 탄착 지점 결정 (맞았으면 ImpactPoint, 안 맞았으면 최대 사거리 지점)
	FVector ImpactPoint = bHit ? MuzzleHitResult.ImpactPoint : FinalMuzzleEnd;

	// 2. 이펙트 멀티캐스트 호출 (Tracer LERP 이동 + 탄피 나이아가라 배출 + Impact 이펙트)
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