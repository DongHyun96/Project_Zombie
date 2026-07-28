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
	m_FireMode = EFireMode::FullAuto;
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
	if (m_bIsFiring || m_bIsReloading) return;
	m_bIsFiring = true;

	// 첫 발 발사
	PlayFireEffects();

	// 사격 모드가 연사일 때만 타이머를 실행
	if (m_FireMode == EFireMode::FullAuto)
	{
		GetWorldTimerManager().SetTimer(m_FireTimerHandle, this, &AC_Rifle::PlayFireEffects, m_FireRate, true);
	}
	else if (m_FireMode == EFireMode::Single)
	{
		m_bIsFiring = false;
	}
}

void AC_Rifle::SwitchFireMode()
{
	// 1. 발사 중이거나 재장전 중일 때는 모드 변경 방지
	if (m_bIsFiring || m_bIsReloading) return;

	// 2. 사격모드 토글 (연사 <-> 단발)
	if (m_FireMode == EFireMode::FullAuto)
	{
		m_FireMode = EFireMode::Single;
	}
	else if (m_FireMode == EFireMode::Single)
	{
		m_FireMode = EFireMode::FullAuto;
	}

	// 3. 변경된 사격모드로 HUD UI 업데이트 (부모 함수 호출)
	Super::SwitchFireMode();

	// 디버그 출력 또는 사운드 재생 (선택)
	UC_Util::Print(FString::Printf(TEXT("FireMode Switched: %d"), (int32)m_FireMode), FColor::Yellow, 2.0f);
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
	// TODO : Reload 모션 방해를 받았다면, 실질적인 재장전 처리를 하면 안됨
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

	FVector CameraStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	// 사거리 설정
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

	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

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

	DrawDebugLine(GetWorld(), MuzzleStart, ActualEndLocation, FColor::Green, false, 0.5f, 0, 1.5f);

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