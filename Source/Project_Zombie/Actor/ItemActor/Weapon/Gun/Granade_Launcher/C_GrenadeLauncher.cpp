// Fill out your copyright notice in the Description page of Project Settings.

#include "C_GrenadeLauncher.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "C_GrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AC_GrenadeLauncher::AC_GrenadeLauncher()
{
	m_FireMode = EFireMode::Single;

	// 서버에서 스폰된 액터를 클라이언트들에게 네트워크 복제
	bReplicates = true;

	// ProjectileMovement의 위치/속도 이동을 네트워크 동기화
	SetReplicateMovement(true);
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

	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0)
		return;

	if (m_OwnerPlayer && m_OwnerPlayer->GetPlayerMoveState() == EPlayerPoseState::Sprint) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	m_CurrentAmmo--;
	if (m_CurrentAmmo < 0) m_CurrentAmmo = 0;
	UpdateAmmoUI();

	PlayFireEffects_Client();

	FVector TargetPoint = GetCameraTargetPoint();
	if (HasAuthority())
	{
		SpawnGrenadeProjectile(TargetPoint);
	}
	else
	{
		Server_ExecuteFire(TargetPoint, nullptr);
	}

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_GrenadeLauncher::ResetFireCooldown, m_FireRate, false);
}

void AC_GrenadeLauncher::Server_ExecuteFire_Implementation(FVector_NetQuantize ImpactPoint, AActor* HitActor)
{
	// 서버에서 클라이언트가 전달한 에임 타겟 지점으로 유탄 스폰
	SpawnGrenadeProjectile(FVector(ImpactPoint));
}

void AC_GrenadeLauncher::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}

void AC_GrenadeLauncher::PlayFireEffects_Client()
{
	Super::PlayFireEffects_Client();
}

void AC_GrenadeLauncher::SpawnShellEject()
{
	// 발사 시 탄피 배출 무력화 (재장전 시 일괄 배출)
}

void AC_GrenadeLauncher::SpawnGrenadeProjectile(const FVector& TargetPoint)
{
	if (!HasAuthority() || !m_WeaponMesh || !GetWorld() || !m_GrenadeClass)
		return;

	FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector LaunchDirection = (TargetPoint - StartLocation).GetSafeNormal();

	FRotator SpawnRotation = LaunchDirection.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = m_OwnerPlayer ? Cast<APawn>(m_OwnerPlayer) : Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AC_GrenadeProjectile* Grenade = GetWorld()->SpawnActor<AC_GrenadeProjectile>(
		m_GrenadeClass,
		StartLocation,
		SpawnRotation,
		SpawnParams
	);

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

void AC_GrenadeLauncher::BeginPlay()
{
	Super::BeginPlay();
}

void AC_GrenadeLauncher::StartReload()
{
	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading)
		return;

	ReleaseTrigger();
	m_bIsReloading = true;

	float ReloadDuration = 2.8f;
	if (m_ReloadAnimation)
	{
		ReloadDuration = m_ReloadAnimation->GetPlayLength();
	}

	if (!HasAuthority())
	{
		Server_StartReload();
	}
	else
	{
		int32 SpentShellCount = m_MaxAmmo - m_CurrentAmmo;
		Multicast_EjectAllSpentShells(SpentShellCount);

		GetWorldTimerManager().SetTimer(m_ReloadTimerHandle, this, &AC_GrenadeLauncher::CompleteReload, ReloadDuration, false);
	}

	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation);
	}

	GetWorldTimerManager().SetTimer(m_ReloadTimerHandle, this, &AC_GrenadeLauncher::CompleteReload, ReloadDuration, false);
}

void AC_GrenadeLauncher::Server_StartReload_Implementation()
{
	Super::Server_StartReload_Implementation();

	int32 SpentShellCount = m_MaxAmmo - m_CurrentAmmo;
	if (SpentShellCount > 0)
	{
		Multicast_EjectAllSpentShells(SpentShellCount);
	}

	float ReloadDuration = 2.8f;
	if (m_ReloadAnimation)
	{
		ReloadDuration = m_ReloadAnimation->GetPlayLength();
	}

	GetWorldTimerManager().SetTimer(m_ReloadTimerHandle, this, &AC_GrenadeLauncher::CompleteReload, ReloadDuration, false);
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

void AC_GrenadeLauncher::CompleteReload()
{
	m_bIsReloading = false;

	m_CurrentAmmo = m_MaxAmmo;
	UpdateAmmoUI();
}

void AC_GrenadeLauncher::AIFire(const FVector& TargetLocation)
{
	if (!HasAuthority() || !m_WeaponMesh || !m_GrenadeClass) return;

	m_CurrentAmmo = FMath::Max(0, m_CurrentAmmo - 1);

	SpawnGrenadeProjectile(TargetLocation);

	PlayFireEffects_Client();
}