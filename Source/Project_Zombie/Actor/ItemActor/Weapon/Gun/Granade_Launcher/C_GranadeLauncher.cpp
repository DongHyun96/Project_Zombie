// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GranadeLauncher.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
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

void AC_GranadeLauncher::EjectAllSpentShells()
{
	if (m_SpentShellCount <= 0)
		return;

	// 누적된 사용 탄피 개수만큼 반복
	for (int32 i = 0; i < m_SpentShellCount; ++i)
	{
		SpawnShellEject();
	}

	// 카운트 초기화
	m_SpentShellCount = 0;
}

void AC_GranadeLauncher::SpawnGrenadeProjectile()
{
	if (!m_WeaponMesh || !GetWorld() || !m_GrenadeClass) return;

	FVector SpawnLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FRotator SpawnRotation = m_WeaponMesh->GetSocketRotation(TEXT("MuzzleFlash"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = m_OwnerPlayer; // 발사한 플레이어 지정 (스플래시 데미지 주체 전달용)

	GetWorld()->SpawnActor<AActor>(m_GrenadeClass, SpawnLocation, SpawnRotation, SpawnParams);
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

	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
}