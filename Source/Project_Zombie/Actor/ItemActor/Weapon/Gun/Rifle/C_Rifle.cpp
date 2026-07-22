// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Rifle.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "../../WeaponComponent/GunComponent/C_GunDataTableComponent.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

AC_Rifle::AC_Rifle()
{

}

bool AC_Rifle::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 첫 눌렸을 시, 동작 처리
	if (nullptr == _WeaponUser)
		return false;

	PullTrigger();

	return true;
}

bool AC_Rifle::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 눌리고 있을 때의 동작 처리 (ex, 연발 사격 처리 등)
	return false;
}

bool AC_Rifle::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 떼었을 때 시점의 동작 처리(딱히 필요없으면 그냥 FireEnd 함수 Gun에서 지우시면 됩니다(동현))
	if (nullptr == _WeaponUser)
	{
		return false;
	}
	else
	{
		ReleaseTrigger();
		return true;
	}
}

bool AC_Rifle::Reload(AC_BasicPlayer* _WeaponUser)
{
	// TODO : Reload 처리
	if (nullptr == _WeaponUser)
	{
		return false;
	}
	else
	{
		Gun_Reload();

		if (m_bIsReloading)
		{
			_WeaponUser->PlayAnimMontage(m_PlayerReloadAnimation);
		}

		return true;
	}
}

void AC_Rifle::PullTrigger()
{

	if (m_bIsFiring || m_bIsReloading) return; // 이미 쏘고 있거나 재장전 중이면 중복 실행 방지
	m_bIsFiring = true;

	// 누르자마자 딜레이 없이 즉시 한 발 발사
	PlayFireEffects();

	// m_FireRate(연사 속도) 간격으로 PlayFireEffects 함수를 무한 반복 호출
	// 마지막 인자인 true가 반복
	if (m_bIsFiring)
	{
		GetWorldTimerManager().SetTimer(m_FireTimerHandle, this, &AC_Rifle::PlayFireEffects, m_FireRate, true);
	}
}

void AC_Rifle::ReleaseTrigger()
{
	// 작동 중이던 연사 타이머 중지
	GetWorldTimerManager().ClearTimer(m_FireTimerHandle);
	m_bIsFiring = false;
}

void AC_Rifle::Gun_Reload()
{
	ReleaseTrigger();

	if (m_CurrentAmmo == m_MaxAmmo || m_bIsReloading)
		return;

	m_bIsReloading = true; // 재장전 시작

	// 재장전 애니메이션 재생
	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	float ReloadDuration = 2.0f;

	if (m_ReloadAnimation)
	{
		ReloadDuration = m_ReloadAnimation->GetPlayLength(); // 애니메이션 실제 길이 추출
	}

	// 2초 타이머 후 탄창만큼의 탄약 보충
	FTimerHandle ReloadTimerHandle;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AC_Rifle::CompleteReload, ReloadDuration, false);
}

void AC_Rifle::CompleteReload()
{
	m_CurrentAmmo = m_MaxAmmo;
	m_bIsReloading = false; // 재장전 완료

	// 새로 장전된 장탄수 UI 업데이트
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
		UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
}

// 총알 소모 로직 후 애니메이션 실행 함수
void AC_Rifle::PlayFireEffects()
{
	if (!ConsumeAmmo())
	{
		return;
	}

	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		// PlayAnimMontage 재생
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}

	// 총기 발사 애니메이션 재생
	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	SpawnShellEject();
	ProcessLineTraceDamage(m_Damage);

}