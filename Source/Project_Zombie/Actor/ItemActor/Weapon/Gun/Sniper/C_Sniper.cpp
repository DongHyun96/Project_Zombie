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

	m_WeaponUser = _WeaponUser;

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

	m_WeaponUser = _WeaponUser;
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
	if (m_WeaponUser && m_PlayerFireAnimation)
	{
		m_WeaponUser->PlayAnimMontage(m_PlayerFireAnimation);
	}

	// 총기 메시 자체 반동 애니메이션
	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	// 탄피 배출 (스나이퍼용 대형 탄피)
	SpawnShellEject();

	// 스나이퍼 발사 (직진 정밀 사격)
	ProcessSniperShot(m_BaseDamage);
}

void AC_Sniper::ProcessSniperShot(float DamageVal)
{
	if (!m_WeaponMesh || !GetWorld()) return;

	FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ForwardVector = m_WeaponMesh->GetSocketRotation(TEXT("MuzzleFlash")).Vector();

	// 사거리 100m (10,000 unit) 정밀 직진 라인트레이스
	FVector MaxEndLocation = StartLocation + (ForwardVector * 10000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_WeaponUser);

	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, MaxEndLocation, ECC_Visibility, QueryParams);
	FVector ActualEndLocation = bHasHit ? HitResult.ImpactPoint : MaxEndLocation;

	DrawDebugLine(GetWorld(), StartLocation, ActualEndLocation, FColor::Green, false, 0.5f, 0, 1.5f);

	if (bHasHit)
	{
		DrawDebugSphere(GetWorld(), ActualEndLocation, 5.0f, 8, FColor::Red, false, 0.5f, 0, 1.5f);

		if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(HitResult.GetActor()))
		{
			UGameplayStatics::ApplyDamage(Enemy, DamageVal, m_WeaponUser->GetController(), this, nullptr);
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
	if (m_WeaponUser && m_PlayerReloadAnimation)
	{
		m_WeaponUser->PlayAnimMontage(m_PlayerReloadAnimation);
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