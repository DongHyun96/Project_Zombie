#include "C_GrenadeProjectile.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AC_GrenadeProjectile::AC_GrenadeProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(8.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComp;

	bAlwaysRelevant = true;
	bReplicates = true;
	SetReplicatingMovement(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	ProjectileMesh->SetupAttachment(RootComponent);

	m_MaxDamage = 100.0f;
	m_MinDamage = 30.0f;
	m_ExplosionRadius = 300.0f;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.8f;

	m_ExplosionEffectScale = 1.0f;
}

void AC_GrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentHit.AddDynamic(this, &AC_GrenadeProjectile::OnHit);

	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	// 스폰처리 하자마자 폭파처리를 해야하는 경우(ex - Muzzle Awareness)
	if (HasAuthority() && m_bHasToExplodeOnSpawn)
		OnExplodeStart();
}

void AC_GrenadeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (m_bHasExploded) return;

	if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
		OnExplodeStart();
}

bool AC_GrenadeProjectile::Server_RequestExplode_Validate(FVector ExplosionLocation)
{
	return true;
}

void AC_GrenadeProjectile::Server_RequestExplode_Implementation(FVector ExplosionLocation)
{
	ExplodeInternal(ExplosionLocation);
}

void AC_GrenadeProjectile::ExplodeInternal(FVector ExplosionLocation)
{
	if (!HasAuthority()) return;

	Multicast_PlayExplosionFX(ExplosionLocation);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams OverlapParams;
	OverlapParams.AddIgnoredActor(this);

	TArray<FOverlapResult> OverlapResults;
	bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(m_ExplosionRadius),
		OverlapParams
	);

	if (bHasOverlap)
	{
		TSet<AActor*> DamagedActors;
		AController* InstigatorController = GetInstigator() ? GetInstigator()->GetController() : nullptr;

		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* Target = Result.GetActor();

			if (!Target || DamagedActors.Contains(Target)) continue;

			float Distance = FVector::Distance(ExplosionLocation, Target->GetActorLocation());
			float AppliedDamage = FMath::Lerp(m_MaxDamage, m_MinDamage, FMath::Clamp(Distance / m_ExplosionRadius, 0.0f, 1.0f));

			UGameplayStatics::ApplyDamage(
				Target,
				AppliedDamage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);

			DamagedActors.Add(Target);
		}
	}

	SetLifeSpan(0.1f);
}

void AC_GrenadeProjectile::OnExplodeStart()
{
	m_bHasExploded = true;

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}

	const FVector HitLocation = GetActorLocation();

	if (m_ExplosionEffect)
	{
		float EffectScale = (m_ExplosionEffectScale > 0.0f) ? m_ExplosionEffectScale : 1.0f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ExplosionEffect, HitLocation, FRotator::ZeroRotator, FVector(EffectScale));
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), m_ExplosionSound, GetActorLocation());
	}

	if (HasAuthority())
	{
		ExplodeInternal(HitLocation);
	}
	else
	{
		Server_RequestExplode(HitLocation);
	}
}

void AC_GrenadeProjectile::Multicast_PlayExplosionFX_Implementation(FVector ExplosionLocation)
{
	if (GetInstigator() && GetInstigator()->IsLocallyControlled()) return;

	if (m_ExplosionEffect)
	{
		float EffectScale = (m_ExplosionEffectScale > 0.0f) ? m_ExplosionEffectScale : 1.0f;

		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			m_ExplosionEffect,
			ExplosionLocation,
			FRotator::ZeroRotator,
			FVector(EffectScale)
		);
		
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), m_ExplosionSound, GetActorLocation());
	}
}
