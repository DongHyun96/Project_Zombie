// Fill out your copyright notice in the Description page of Project Settings.

#include "C_GrenadeLauncher.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "C_GrenadeProjectile.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AC_GrenadeLauncher::AC_GrenadeLauncher()
{
	m_FireMode = EFireMode::Single;

	// 서버에서 스폰된 액터를 클라이언트들에게 네트워크 복제
	bReplicates = true;

	// ProjectileMovement의 위치/속도 이동을 네트워크 동기화
	SetReplicateMovement(true);

}

void AC_GrenadeLauncher::Multicast_EjectAllSpentShells_Implementation(int32 SpentShellCount)
{
	if (SpentShellCount <= 0 || !m_ShellEjectNiagaraSystem || !m_ShellMesh || !m_WeaponMesh || !GetWorld())
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
		NiagaraComp->SetIntParameter(FName("ShellCount"), SpentShellCount);
	}
}

bool AC_GrenadeLauncher::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	if (m_bIsReloading)
		return false;

	PullTrigger();
	return true;
}

bool AC_GrenadeLauncher::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_GrenadeLauncher::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	ReleaseTrigger();
	return true;
}

bool AC_GrenadeLauncher::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (nullptr == _WeaponUser)
		return false;

	StartReload();
	return true;
}

void AC_GrenadeLauncher::PullTrigger()
{
	// 탄약이 없거나, 이미 발사 중이거나, 재장전 중이거나, 쿨타임 중이면 리턴
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0)
		return;

	m_bIsFiring = true;
	m_bCanFire = false;

	// 1. 로컬 플레이어 사격 연출 (부모의 애니메이션 재생)
	PlayFireEffects_Local();

	// 2. 서버에 사격 요청
	Server_PullTrigger();

	// 3. 발사 쿨타임 타이머 설정
	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_GrenadeLauncher::ResetFireCooldown, m_FireRate, false);
}

void AC_GrenadeLauncher::ReleaseTrigger()
{
	m_bIsFiring = false;

	if (!HasAuthority())
	{
		Server_ReleaseTrigger();
	}
}

void AC_GrenadeLauncher::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

// [서버] 부모(AC_GunBase)의 Server_PullTrigger_Implementation 내부에서 호출되는 함수
void AC_GrenadeLauncher::Server_ExecuteFire()
{
	// 1. 조준점(ImpactPoint) 계산
	FVector TargetPoint = GetCameraTargetPoint();

	// 2. 서버에서 유탄 액터 스폰
	SpawnGrenadeProjectile(TargetPoint);

	// 3. 다른 플레이어들에게 발사 이펙트 전파
	Multicast_PlayFireEffects(TargetPoint);
}

// 부모(AC_GunBase)의 PlayFireEffects_Local()을 그대로 따름
void AC_GrenadeLauncher::PlayFireEffects_Local()
{
	Super::PlayFireEffects_Local();
}

void AC_GrenadeLauncher::SpawnShellEject()
{
	// 발사 시 탄피 배출 무력화
}

void AC_GrenadeLauncher::SpawnGrenadeProjectile(const FVector& TargetPoint)
{
	// 스폰 로직은 오직 서버에서만 작동
	if (!HasAuthority() || !m_WeaponMesh || !GetWorld() || !m_GrenadeClass || !m_OwnerPlayer)
		return;

	FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector LaunchDirection = (TargetPoint - StartLocation).GetSafeNormal();

	float LaunchSpeed = 2500.0f;
	FVector LaunchVelocity = LaunchDirection * LaunchSpeed;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = m_OwnerPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

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

FVector AC_GrenadeLauncher::GetCameraTargetPoint() const
{
	if (!m_OwnerPlayer)
		return FVector::ZeroVector;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return m_OwnerPlayer->GetActorLocation() + (m_OwnerPlayer->GetActorForwardVector() * 5000.0f);
	}

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();

	float AimDistance = 5000.0f;
	FVector CameraEnd = CameraLocation + (CameraForward * AimDistance);

	FHitResult CameraHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
		CameraHitResult,
		CameraLocation,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	return bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;
}

void AC_GrenadeLauncher::StartReload()
{
	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading)
		return;

	ReleaseTrigger();
	m_bIsReloading = true;

	//  서버라면 직접 탄피 배출 멀티캐스트 호출 / 클라이언트라면 서버 RPC 호출
	if (HasAuthority())
	{
		int32 SpentShellCount = m_MaxAmmo - m_CurrentAmmo;
		Multicast_EjectAllSpentShells(SpentShellCount);
	}
	else
	{
		Server_StartReload();
	}

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

	GetWorldTimerManager().SetTimer(m_ReloadTimerHandle, this, &AC_GrenadeLauncher::CompleteReload, ReloadDuration, false);
}

void AC_GrenadeLauncher::Server_StartReload_Implementation()
{
	Super::Server_StartReload_Implementation();

	// 서버가 클라이언트의 요청을 받아 모든 플레이어에게 탄피 배출 멀티캐스트 전파
	int32 SpentShellCount = m_MaxAmmo - m_CurrentAmmo;
	if (SpentShellCount > 0)
	{
		Multicast_EjectAllSpentShells(SpentShellCount);
	}
}

void AC_GrenadeLauncher::CompleteReload()
{
	m_bIsReloading = false;

	// 탄약 채우기는 서버에서 처리
	if (HasAuthority())
	{
		m_CurrentAmmo = m_MaxAmmo;

		// 서버(호스트) 본인 화면의 HUD 갱신
		if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
		{
			OnRep_CurrentAmmo();
		}
	}
}