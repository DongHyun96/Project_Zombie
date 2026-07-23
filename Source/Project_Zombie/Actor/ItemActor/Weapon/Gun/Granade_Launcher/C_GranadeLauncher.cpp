// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GranadeLauncher.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "C_GrenadeProjectile.h"

#include "GameModeAndManager/C_UIManager.h"

#include "UI/MainHUD/C_GameMainHUD.h"

#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AC_GranadeLauncher::AC_GranadeLauncher()
{

}

bool AC_GranadeLauncher::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	//m_OwnerPlayer = _WeaponUser;

	if (m_bIsReloading)
		return false;

	PullTrigger();
	return true;
}

bool AC_GranadeLauncher::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_GranadeLauncher::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	ReleaseTrigger();
	return true;
}

bool AC_GranadeLauncher::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	//m_OwnerPlayer = _WeaponUser;
	StartReload();

	return true;
}

void AC_GranadeLauncher::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	PlayFireEffects();

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_GranadeLauncher::ResetFireCooldown, m_FireRate, false);
}

void AC_GranadeLauncher::ReleaseTrigger()
{
	m_bIsFiring = false;
}

void AC_GranadeLauncher::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_GranadeLauncher::PlayFireEffects()
{
	if (!ConsumeAmmo())
	{
		ReleaseTrigger();
		m_bCanFire = true;
		return;
	}

	m_SpentShellCount++;

	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}

	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	// 유탄 액터 스폰
	SpawnGrenadeProjectile();
}

void AC_GranadeLauncher::SpawnGrenadeProjectile()
{
	if (!m_WeaponMesh || !GetWorld() || !m_GrenadeClass || !m_OwnerPlayer)
		return;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager)
		return;

	// =========================================================================
	// [1단계] 전방 충돌체(벽/지형/적) 감지 라인트레이스
	// =========================================================================
	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float MaxTraceRange = 10000.0f; // 최대 탐색 거리 (충돌체 감지를 위해 충분히 길게 설정)
	FVector CameraEnd = CameraStart + (CameraForward * MaxTraceRange);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);          // 총 자체 무시
	QueryParams.AddIgnoredActor(m_OwnerPlayer);   // 플레이어 자신 무시

	// ECC_Visibility 채널을 사용해 화면에 보이는 모든 충돌체(벽, 적, 오브젝트 등)를 감지
	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
		CameraHitResult,
		CameraStart,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	// ★ 전방에 부딪힌 충돌체(ImpactPoint)가 있으면 그 지점을 목표로, 없으면 허공 끝점 지정
	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	// =========================================================================
	// [2단계] 근접 충돌 예외 처리 (선택형 보정)
	// =========================================================================
	FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));

	// 만약 내 바로 앞(예: 1m 이내)에 벽이 바짝 붙어있는 경우, 포물선 계산이 꼬이는 것을 방지
	float DistanceToTarget = FVector::Distance(StartLocation, TargetPoint);

	FVector OutLaunchVelocity = FVector::ZeroVector;
	float LaunchSpeed = 2500.0f; // 유탄 기본 속도

	// [3단계] SuggestProjectileVelocity로 충돌체 지점까지의 포물선 속도 계산
	bool bHaveValidSolution = false;

	// 근묵자흑처럼 너무 가깝지 않을 때만 포물선 역산 실행
	if (DistanceToTarget > 100.0f)
	{
		bHaveValidSolution = UGameplayStatics::SuggestProjectileVelocity(
			this,
			OutLaunchVelocity,      // [Out] 계산된 포물선 속도 Vector
			StartLocation,          // 출발점 (총구)
			TargetPoint,            // 충돌 감지된 표적점 (Hit Location)
			LaunchSpeed,            // 발사 속도
			false,                  // HighArc (false: 낮고 빠른 포물선)
			0.0f,                   // CollisionRadius
			0.0f,                   // OverrideGravityZ (월드 중력 적용)
			ESuggestProjVelocityTraceOption::DoNotTrace
		);
	}

	// 충돌체와의 거리가 너무 가깝거나/멀어서 포물선 해(Solution)가 나오지 않을 때는
	// 충돌체 지점을 향해 직사(직선)로 발사
	if (!bHaveValidSolution)
	{
		OutLaunchVelocity = (TargetPoint - StartLocation).GetSafeNormal() * LaunchSpeed;
	}

	// =========================================================================
	// [4단계] 유탄 액터 스폰 및 계산된 Velocity(속도) 적용
	// =========================================================================
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = m_OwnerPlayer;

	FRotator SpawnRotation = OutLaunchVelocity.Rotation();

	AC_GrenadeProjectile* Grenade = GetWorld()->SpawnActor<AC_GrenadeProjectile>(
		m_GrenadeClass,
		StartLocation,
		SpawnRotation,
		SpawnParams
	);

	if (Grenade && Grenade->GetProjectileMovement())
	{
		Grenade->GetProjectileMovement()->Velocity = OutLaunchVelocity;
	}
}

void AC_GranadeLauncher::CompleteReload()
{
	m_CurrentAmmo = m_MaxAmmo;
	m_bIsReloading = false;

	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	{
		if (UIManager->GetMainHUDWidget())
		{
			UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
		}
	}
}