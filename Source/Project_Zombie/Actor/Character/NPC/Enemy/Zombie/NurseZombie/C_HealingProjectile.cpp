// Fill out your copyright notice in the Description page of Project Settings.


#include "C_HealingProjectile.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_NurseZombie.h"

#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Utility/C_Util.h"


AC_HealingProjectile::AC_HealingProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	m_MainCollider = CreateDefaultSubobject<USphereComponent>(TEXT("MainCollider"));
	m_MainCollider->InitSphereRadius(30.f);
	
	SetRootComponent(m_MainCollider);

	m_ProjectileMovement								= CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	m_ProjectileMovement->bRotationFollowsVelocity		= true;
	m_ProjectileMovement->bShouldBounce					= false;
	m_ProjectileMovement->Bounciness					= 0.f;
	m_ProjectileMovement->bIsHomingProjectile			= true;
	m_ProjectileMovement->HomingAccelerationMagnitude	= 1500.f;
	m_ProjectileMovement->ProjectileGravityScale		= 0.f;
	
	m_NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	m_NiagaraComponent->SetupAttachment(m_MainCollider);
}

void AC_HealingProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 나이아가라 재생
	if (m_NiagaraComponent && m_TrailEffect)
		m_NiagaraComponent->SetAsset(m_TrailEffect);

	m_MainCollider->OnComponentHit.AddDynamic(this, &AC_HealingProjectile::OnMainColliderHit);
	m_MainCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_HealingProjectile::OnMainColliderBeginOverlap);
	
	Deactivate();
}

void AC_HealingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AC_HealingProjectile::Fire
(
	const FVector& 	_FireStartLocation,
	const FVector& 	_FireDirection,
	AC_NurseZombie*	_SpawnedBy,
	AC_BasicEnemy* 	_HealingTarget,
	float			_TotalHealAmount
)
{
	// Invalid Target
	if (!_HealingTarget) return false;
	
	// 이미 사망한 HealingTarget인 경우
	if (_HealingTarget->GetStatComponent()->GetCurHP() <= 0.f)
	{
		UC_Util::Print("HealingTarget already dead", FColor::Red, 10.f);
		return false;
	}
	
	/* 스폰 처리 */

	SetActorLocation(_FireStartLocation);
	m_bActive         = true;
	m_SpawnedBy       = _SpawnedBy;
	m_HealTarget      = _HealingTarget;
	m_FireDirection   = _FireDirection.GetSafeNormal();
	m_TotalHealAmount = _TotalHealAmount;

	// HealingTarget 사망 시, 호출받을 Deactivate 처리 및 Pool로 다시 되돌아가기 함수 Delegate 구독
	m_HealTarget->GetStatComponent()->OnCurHPReachedZeroDelegate.AddUObject(this, &AC_HealingProjectile::OnHealTargetDead);
	
	// Projectile 활성화 및 HomingTarget 설정
	m_ProjectileMovement->HomingTargetComponent = _HealingTarget->GetRootComponent();
	m_ProjectileMovement->Velocity              = m_Speed * m_FireDirection;
	m_ProjectileMovement->bSimulationEnabled    = true;
	m_ProjectileMovement->SetUpdatedComponent(GetRootComponent());
	
	SetActorHiddenInGame(false);
	m_NiagaraComponent->Activate();
	
	
	m_MainCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UC_Util::Print("Fire succeeded", FColor::Red, 10.f);
	return true;
}

void AC_HealingProjectile::Deactivate()
{
	m_MainCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);

	if (m_HealTarget)
	{
		// HealTarget이 존재했었다면, StaatComponent OnCurHPReachedZero Delegate 구독 해제
		m_HealTarget->GetStatComponent()->OnCurHPReachedZeroDelegate.RemoveAll(this);
		m_HealTarget = nullptr;
	}
	
	m_bActive		  = false;
	m_SpawnedBy       = nullptr;
	m_FireDirection   = FVector::ZeroVector;
	m_TotalHealAmount = 0.f;
	
	m_ProjectileMovement->Velocity              = FVector::ZeroVector;
	m_ProjectileMovement->bSimulationEnabled    = false;
	m_ProjectileMovement->HomingTargetComponent = nullptr;
	m_ProjectileMovement->SetUpdatedComponent(nullptr);
	// m_ProjectileMovement->Deactivate();
	
	// TrailEffect 비활성화
	// m_NiagaraComponent->DeactivateImmediate();
	m_NiagaraComponent->Deactivate();
}

void AC_HealingProjectile::OnHealTargetDead(AC_BasicCharacter* _DeadCharacter)
{
	// 해당하는 HealTarget이 이미 사망한 경우, 그리고 이 HealingProjectile이 아직 살아있는 경우 다시 Pool로 돌아가는 처리
	if (m_bActive)
	{
		Deactivate();
		ZOMBIE_MANAGER->ReturnHealingProjectileToPool(this);
	}
}

void AC_HealingProjectile::OnMainColliderBeginOverlap
(
	UPrimitiveComponent* _OverlapComponent,
	AActor*				 _OtherActor,
	UPrimitiveComponent* _OtherComp,
	int32				 _OtherBodyIndex,
	bool				 _bFromSweep,
	const FHitResult&	 _SweepResult
)
{
	AC_BasicEnemy* OverlappedEnemy = Cast<AC_BasicEnemy>(_OtherActor);
	
	if (!OverlappedEnemy) // Enemy가 아닌 다른 물체와의 충돌 (지형지물 등) -> 바로 Pool로 돌아가기
	{
		Deactivate();
		ZOMBIE_MANAGER->ReturnHealingProjectileToPool(this); // 좀비 풀로 다시 들어가기
		return;
	}

	// 이 Projectile을 스폰 시킨 NurseZombie에게 맞았을 경우, continue
	if (OverlappedEnemy == m_SpawnedBy) return;
	
	/*
	 * Enemy와의 충돌이 일어남 -> 해당 Enemy가 풀피라면 해당 충돌을 무시하고 continue
	 * 충돌한 Enemy가 풀피이고, HealingTarget이면 그냥 종료 | 충돌한 Enemy가 풀피이고, HealingTarget이 아니라면 continue
	 * 만약 풀피가 아니라면 HomingProjectile target 여부에 관계없이 해당 Enemy 힐 처리
	 */

	UC_StatComponentBase* OveralppedStatCom = OverlappedEnemy->GetStatComponent(); 
	if (OveralppedStatCom->IsCurHPFull())
	{
		if (OverlappedEnemy != m_HealTarget) return; // Enemy긴 한데 아직 Target에 도달하지 못함
		
		// HealTarget에 도달함 -> 풀피라 힐 처리 x -> Pool로 다시 되돌아가기
		Deactivate();
		ZOMBIE_MANAGER->ReturnHealingProjectileToPool(this);
		return;
	}
	
	// 힐 처리 및 Pool로 돌아가기
	OveralppedStatCom->IncreaseCurHP(m_TotalHealAmount);
	
	if (m_HealEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation
		(
			GetWorld(),
			m_HealEffect,
			GetActorLocation()
		);
	}
	
	Deactivate();
	ZOMBIE_MANAGER->ReturnHealingProjectileToPool(this);
}

void AC_HealingProjectile::OnMainColliderHit
(
	UPrimitiveComponent* _HittedComponent,
	AActor*				 _OtherActor,
	UPrimitiveComponent* _OtherComp,
	FVector				 _NormalImpulse,
	const FHitResult&	 _HitResult
)
{
	// 지형지물 및 기타 불필요한 물체와의 충돌 -> Pool로 바로 들어가기 처리
	Deactivate();
	ZOMBIE_MANAGER->ReturnHealingProjectileToPool(this);
}
