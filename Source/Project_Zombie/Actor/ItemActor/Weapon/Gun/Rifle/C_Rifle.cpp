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
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
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
	RifleLineTraceDamage(m_Damage, m_SpreadAngle);

}

void AC_Rifle::RifleLineTraceDamage(float DamageVal, float SpreadAngleDegree)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer)
		return;

	// 1. 플레이어의 컨트롤러(카메라) 가져오기
	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager)
		return;

	// =========================================================================
	// [1단계] 카메라 중앙에서 1차 라인트레이스를 쏴서 '크로스헤어가 가리키는 타겟' 찾기
	// =========================================================================
	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	// 사거리 설정 (기존 코드의 3500.f 사용)
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

	// 카메라 레이저가 맞은 진짜 목표 지점 (안 맞았으면 사거리 끝 지점)
	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;

	// =========================================================================
	// [2단계] 실제 총구(MuzzleFlash)에서 타겟 지점(TargetPoint)을 향해 사격
	// =========================================================================
	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));

	// 총구 ➔ TargetPoint 로 향하는 사격 방향 벡터 계산
	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	// 탄돌림(탄착군 오차) 적용
	if (SpreadAngleDegree > 0.0f)
	{
		float ConeHalfAngleRad = FMath::DegreesToRadians(SpreadAngleDegree);
		ShootDirection = FMath::VRandCone(ShootDirection, ConeHalfAngleRad);
	}

	FVector FinalMuzzleEnd = MuzzleStart + (ShootDirection * TraceRange);

	// 총구 기준 2차 라인트레이스 (실제 데미지 판단용)
	FHitResult MuzzleHitResult;
	bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(
		MuzzleHitResult,
		MuzzleStart,
		FinalMuzzleEnd,
		ECC_Visibility,
		QueryParams
	);

	FVector ActualEndLocation = bMuzzleHit ? MuzzleHitResult.ImpactPoint : FinalMuzzleEnd;

	// Debug 그린 선 (총구 ➔ 실제 타격 지점)
	DrawDebugLine(GetWorld(), MuzzleStart, ActualEndLocation, FColor::Green, false, 0.5f, 0, 1.5f);

	// =========================================================================
	// [3단계] 데미지 적용
	// =========================================================================
	if (bMuzzleHit)
	{
		DrawDebugSphere(GetWorld(), ActualEndLocation, 7.0f, 12, FColor::Red, false, 0.5f, 0, 1.5f);

		if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(MuzzleHitResult.GetActor()))
		{
			AController* InstigatorController = m_OwnerPlayer->GetController();
			UGameplayStatics::ApplyDamage(Enemy, DamageVal, InstigatorController, this, nullptr);
		}
	}
}