// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Sniper.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Kismet/GameplayStatics.h"

AC_Sniper::AC_Sniper()
{

}

bool AC_Sniper::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	//m_WeaponUser = _WeaponUser;

	// 재장전 중 사격 키 누르면 사격 차단
	if (m_bIsReloading)
		return false;

	PullTrigger();
	return true;
}

bool AC_Sniper::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_Sniper::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	ReleaseTrigger();
	return true;
}

bool AC_Sniper::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	//m_OwnerPlayer = _WeaponUser;
	StartReload();

	return true;
}

void AC_Sniper::PullTrigger()
{
	// 이미 사격 중, 재장전 중, 쿨타임 진행 중이면 사격 불가
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire) return;

	m_bIsFiring = true;
	m_bCanFire = false; // 쿨타임 시작

	PlayFireEffects();

	// m_FireRate 동안 딜레이 후 다음 사격 가능하도록 설정
	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_Sniper::ResetFireCooldown, m_FireRate, false);
}

void AC_Sniper::ReleaseTrigger()
{
	m_bIsFiring = false;
}

void AC_Sniper::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_Sniper::PlayFireEffects()
{
	// 부모의 ConsumeAmmo 호출 (탄약 부족 시 ReleaseTrigger)
	if (!ConsumeAmmo())
	{
		ReleaseTrigger();
		m_bCanFire = true;
		return;
	}

	// 플레이어 사격/반동 몽타주
	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}

	// 총기 메시 자체 반동 애니메이션
	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	// 탄피 배출 (스나이퍼용 대형 탄피)
	SpawnShellEject();

	// 스나이퍼 발사 (직진 정밀 사격)
	ProcessSniperShot(m_Damage);
}

void AC_Sniper::ProcessSniperShot(float DamageVal)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer)
		return;

	// 1. 플레이어 컨트롤러(카메라) 확인
	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager)
		return;

	// =========================================================================
	// [1단계] 카메라 정중앙(크로스헤어/스코프)에서 1차 라인트레이스로 주 타겟점 구하기
	// =========================================================================
	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float TraceRange = 10000.0f; // 스나이퍼 사거리 100m
	FVector CameraEnd = CameraStart + (CameraForward * TraceRange);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
		CameraHitResult,
		CameraStart,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	// 스코프/크로스헤어가 가리키는 정확한 목표 위치
	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	// =========================================================================
	// [2단계] 총구(MuzzleFlash)에서 TargetPoint를 향해 정밀 사격
	// =========================================================================
	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	FVector FinalMuzzleEnd = MuzzleStart + (ShootDirection * TraceRange);

	FHitResult MuzzleHitResult;
	bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(
		MuzzleHitResult,
		MuzzleStart,
		FinalMuzzleEnd,
		ECC_Visibility,
		QueryParams
	);

	FVector ActualEndLocation = bMuzzleHit ? MuzzleHitResult.ImpactPoint : FinalMuzzleEnd;

	// 스나이퍼 궤적 디버그 선 (총구 ➔ 명중 지점)
	DrawDebugLine(GetWorld(), MuzzleStart, ActualEndLocation, FColor::Green, false, 0.5f, 0, 1.5f);

	// =========================================================================
	// [3단계] 데미지 적용
	// =========================================================================
	if (bMuzzleHit)
	{
		DrawDebugSphere(GetWorld(), ActualEndLocation, 5.0f, 8, FColor::Red, false, 0.5f, 0, 1.5f);

		if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(MuzzleHitResult.GetActor()))
		{
			AController* InstigatorController = m_OwnerPlayer->GetController();
			UGameplayStatics::ApplyDamage(Enemy, DamageVal, InstigatorController, this, nullptr);
		}
	}
}

void AC_Sniper::StartReload()
{
	ReleaseTrigger();

	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading)
		return;

	m_bIsReloading = true;

	// 총기 재장전 애니메이션
	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	// 플레이어 재장전 몽타주
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation);
	}

	float ReloadDuration = 0.f;

	if (m_ReloadAnimation)
	{
		ReloadDuration = m_ReloadAnimation->GetPlayLength();
	}

	GetWorldTimerManager().SetTimer(m_ReloadTimerHandle, this, &AC_Sniper::CompleteReload, ReloadDuration, false);
}

void AC_Sniper::CompleteReload()
{
	m_CurrentAmmo = m_MaxAmmo;
	m_bIsReloading = false;

	// UI 갱신
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	{
		if (UIManager->GetMainHUDWidget())
		{
			UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
		}
	}
}