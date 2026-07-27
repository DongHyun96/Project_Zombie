// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ShotGun.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AC_ShotGun::AC_ShotGun()
{
	m_PelletCount = 8;				// 총알 갯수
	m_SpreadAngle = 6.0f;			// 집탄률
	m_SingleShellInsertTime = 0.6f; // 1발당 0.6초

	// 실질적으로 점사는 아니지만, UI 표기 상 Burst 이미지 느낌이 ShotGun과 비슷해서 Burst로 넣어둠
	m_FireMode = EFireMode::Burst;
}

bool AC_ShotGun::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	m_OwnerPlayer = _WeaponUser;

	// 재장전 중에 사격 시도 시 재장전을 중단하고 사격 가능하게 처리
	if (m_bIsReloading)
	{
		EndReload();
	}

	PullTrigger();
	return true;
}

bool AC_ShotGun::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_ShotGun::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	ReleaseTrigger();
	return true;
}

bool AC_ShotGun::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	m_OwnerPlayer = _WeaponUser;
	StartReload();

	return true;
}

void AC_ShotGun::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	PlayFireEffects();

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_ShotGun::ResetFireCooldown, m_FireRate, false);
}

void AC_ShotGun::ReleaseTrigger()
{
	m_bIsFiring = false;
}

void AC_ShotGun::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_ShotGun::PlayFireEffects()
{
	if (!ConsumeAmmo())
	{
		ReleaseTrigger();
		m_bCanFire = true;
		return;
	}

	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}

	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	SpawnShellEject();
	ProcessShotgunPellets(m_Damage);
}

void AC_ShotGun::ProcessShotgunPellets(float BaseDamagePerPellet)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer)
		return;

	// 1. 플레이어 컨트롤러(카메라) 확인
	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager)
		return;

	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float TraceRange = 3500.0f;
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

	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector MainDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	for (int32 i = 0; i < m_PelletCount; ++i)
	{
		FVector SpreadDir = MainDirection;
		if (m_SpreadAngle > 0.0f)
		{
			SpreadDir = FMath::VRandCone(MainDirection, FMath::DegreesToRadians(m_SpreadAngle));
		}

		FVector MaxEndLocation = MuzzleStart + (SpreadDir * TraceRange);

		FHitResult PelletHitResult;
		bool bHasHit = GetWorld()->LineTraceSingleByChannel(
			PelletHitResult,
			MuzzleStart,
			MaxEndLocation,
			ECC_Visibility,
			QueryParams
		);

		FVector ActualEndLocation = bHasHit ? PelletHitResult.ImpactPoint : MaxEndLocation;

		// 펠릿 디버그 선 (총구 ➔ 탄착점)
		DrawDebugLine(GetWorld(), MuzzleStart, ActualEndLocation, FColor::Green, false, 0.3f, 0, 1.0f);

		if (bHasHit)
		{
			DrawDebugSphere(GetWorld(), ActualEndLocation, 4.0f, 8, FColor::Red, false, 0.4f, 0, 1.0f);

			if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(PelletHitResult.GetActor()))
			{
				AController* InstigatorController = m_OwnerPlayer->GetController();
				UGameplayStatics::ApplyDamage(Enemy, BaseDamagePerPellet, InstigatorController, this, nullptr);
			}
		}
	}
}

void AC_ShotGun::StartReload()
{
	ReleaseTrigger();

	// 탄약이 이미 꽉 찼거나 이미 재장전 중이면 취소
	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading)
		return;

	m_bIsReloading = true;

	// 첫 1발 즉시 장전 시도 타이머
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
	// 탄약이 꽉 찼으면 재장전 종료
	if (m_CurrentAmmo >= m_MaxAmmo)
	{
		EndReload();
		return;
	}

	m_CurrentAmmo++;

	// UI 갱신
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
	
	// 1발 넣는 몽타주/애니메이션
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation, 1.0f);
	}

	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	// 1발을 넣은 직후 장탄수가 꽉 찼다면 바로 종료
	if (m_CurrentAmmo >= m_MaxAmmo)
	{
		EndReload();
	}
}

void AC_ShotGun::EndReload()
{
	m_bIsReloading = false;

	// 루프 타이머 클리어
	GetWorldTimerManager().ClearTimer(m_ReloadLoopTimer);

	// 플레이어 재장전 몽타주 정지
	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);
	}
}