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

	// m_OwnerPlayer = _WeaponUser;

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

	// m_OwnerPlayer = _WeaponUser;
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

void AC_GranadeLauncher::EjectAllSpentShells()
{
	if (m_SpentShellCount <= 0 || !m_ShellEjectNiagaraSystem || !m_ShellMesh || !m_WeaponMesh || !GetWorld())
		return;

	FTransform EjectTransform = m_WeaponMesh->GetSocketTransform(TEXT("AmmoEject"), RTS_World);

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		m_ShellEjectNiagaraSystem,
		EjectTransform.GetLocation(),
		EjectTransform.GetRotation().Rotator()
	);

	if (NiagaraComp)
	{
		NiagaraComp->SetVariableStaticMesh(FName("ShellMesh"), m_ShellMesh);
		NiagaraComp->SetIntParameter(FName("ShellCount"), m_SpentShellCount);
	}

	m_SpentShellCount = 0;
}

void AC_GranadeLauncher::SpawnGrenadeProjectile()
{
	if (!m_WeaponMesh || !GetWorld() || !m_GrenadeClass || !m_OwnerPlayer)
		return;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager)
		return;

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float AimDistance = 5000.0f;
	FVector CameraEnd = CameraLocation + (CameraForward * AimDistance);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);          // 총 자체 무시
	QueryParams.AddIgnoredActor(m_OwnerPlayer);   // 플레이어 자신 무시

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
		CameraHitResult,
		CameraLocation,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));

	FVector LaunchDirection = (TargetPoint - StartLocation).GetSafeNormal();

	float LaunchSpeed = 2500.0f;
	FVector LaunchVelocity = LaunchDirection * LaunchSpeed;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = m_OwnerPlayer;

	FRotator SpawnRotation = LaunchVelocity.Rotation();

	AC_GrenadeProjectile* Grenade = GetWorld()->SpawnActor<AC_GrenadeProjectile>(
		m_GrenadeClass,
		StartLocation,
		SpawnRotation,
		SpawnParams
	);

	if (Grenade && Grenade->GetProjectileMovement())
	{
		Grenade->GetProjectileMovement()->Velocity = LaunchVelocity;
	}
}

void AC_GranadeLauncher::StartReload()
{
	ReleaseTrigger();

	EjectAllSpentShells();

	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading)
		return;

	m_bIsReloading = true;

	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation);
	}

	float ReloadDuration = 2.8f;
	if (m_ReloadAnimation)
	{
		ReloadDuration = m_ReloadAnimation->GetPlayLength();
	}

	GetWorldTimerManager().SetTimer(m_ReloadTimerHandle, this, &AC_GranadeLauncher::CompleteReload, ReloadDuration, false);
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