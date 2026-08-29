// Fill out your copyright notice in the Description page of Project Settings.

#include "C_GrenadeLauncher.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "C_GrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"

AC_GrenadeLauncher::AC_GrenadeLauncher()
{
	m_FireMode = EFireMode::Single;

	// 서버에서 스폰된 액터를 클라이언트들에게 네트워크 복제
	bReplicates = true;

	// ProjectileMovement의 위치/속도 이동을 네트워크 동기화 -> 를 Projectile에 해두어야 함
	// SetReplicateMovement(true);
}

bool AC_GrenadeLauncher::Reload(AC_BasicPlayer* _WeaponUser)
{
	// 일반적인 재장전 진행 불가 상황
	if (!Super::Reload(_WeaponUser)) return false;

	// 재장전 성공
	
	const int32 SpentShellCount = m_MaxAmmo - m_CurrentAmmo;
	Server_EjectAllSpentShells(SpentShellCount); // 나를 포함한 모든 화면에 SpentShells 내뱉기 처리
	
	return true;
}

void AC_GrenadeLauncher::Server_ExecuteFire_Implementation(FVector_NetQuantize ImpactPoint, AActor* HitActor)
{
	if (m_OwnerPlayer && m_OwnerPlayer->IsDead()) return;
	
	// HitActor의 경우, MuzzleAwareness에 잡힌 Actor를 사용 (없다면 자동적으로 nullptr를 넘겨서 일반 유탄 사격처리로 넘어간다)
	SpawnGrenadeProjectile(FVector(ImpactPoint), HitActor);
	
	// 사격 모션 재생 처리 Multicast로 다른 Player들에게 자기자신의 모습 알림 처리
	// ImpactPoint는 Dummy data
	Multicast_PlayFireEffects(ImpactPoint);
}

void AC_GrenadeLauncher::Multicast_PlayFireEffects_Implementation(FVector_NetQuantize ImpactPoint)
{
	PRINT_LOCAL(GetWorld(), "PlayFireEffects", FColor::Cyan, 10.f);
	
	// 자기 자신의 FireEffect Animation은 이미 재생한 상황
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled()) return;
	PlayFireEffects(); // 다른 Player의 FireEffect Animation 재생 처리
}

void AC_GrenadeLauncher::Server_EjectAllSpentShells_Implementation(int32 SpentShellCount)
{
	Multicast_EjectAllSpentShells(SpentShellCount);
}

void AC_GrenadeLauncher::ResetFireCooldown()
{
	m_bCanFire  = true;
	m_bIsFiring = false;
}

void AC_GrenadeLauncher::SpawnShellEject()
{
	// GrenadeLauncher의 경우 발사 시 탄피 배출 무력화 (재장전 시 일괄 배출)
}

bool AC_GrenadeLauncher::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (!Super::OnStartFire(_WeaponUser)) return false;

	if (!m_bCanFire) return false;

	m_bIsFiring = true;
	m_bCanFire  = false;

	m_CurrentAmmo--;
	if (m_CurrentAmmo < 0) m_CurrentAmmo = 0;
	UpdateAmmoUI();

	// 발사 Animation 재생
	PlayFireEffects();

	// Muzzle Aware 위치라면 해당 TargetPoint를 벽면(또는 물체 등)으로 둠
	const FVector TargetPoint = m_MuzzleAwareActor == nullptr ? GetCameraTargetPoint() : m_MuzzleAwareImpactPoint;
	
	Server_ExecuteFire(TargetPoint, m_MuzzleAwareActor); // 어차피 클라이든, 서버든 처리 같음

	GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_GrenadeLauncher::ResetFireCooldown, m_FireRate, false);
	return true;
}

void AC_GrenadeLauncher::SpawnGrenadeProjectile(const FVector& TargetPoint, AActor* MuzzleAwareActor)
{
	if (!HasAuthority() || !m_WeaponMesh || !GetWorld() || !m_GrenadeClass)
		return;

	
	// Muzzle Aware 거리에 파묻힌 경우, 예외처리로 스폰 및 바로 터쳐버림
	if (MuzzleAwareActor)
	{
		AC_GrenadeProjectile* Grenade = GetWorld()->SpawnActorDeferred<AC_GrenadeProjectile>
		(
			m_GrenadeClass,
			FTransform(TargetPoint),
			this, Cast<APawn>(GetOwner()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
		
		Grenade->SetHasToExplodeOnSpawn(); // 바로 폭파 처리 세팅
		Grenade->FinishSpawning(FTransform(TargetPoint));
		return;
	}
	
	const FVector StartLocation   = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	const FVector LaunchDirection = (TargetPoint - StartLocation).GetSafeNormal();
	const FRotator SpawnRotation  = LaunchDirection.Rotation();
	
	FActorSpawnParameters SpawnParams{};
	SpawnParams.Owner                          = this;
	SpawnParams.Instigator                     = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AC_GrenadeProjectile>
	(
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

void AC_GrenadeLauncher::AIFire(const FVector& TargetLocation)
{
	if (!HasAuthority() || !m_WeaponMesh || !m_GrenadeClass) return;

	m_CurrentAmmo = FMath::Max(0, m_CurrentAmmo - 1);

	SpawnGrenadeProjectile(TargetLocation);

	PlayFireEffects();
}