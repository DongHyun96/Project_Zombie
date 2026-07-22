// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrenadeProjectile.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Actor/ItemActor/Weapon/ThrowableWeapon/Strategy/C_GrenadeExplode.h"

AC_GrenadeProjectile::AC_GrenadeProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(8.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AC_GrenadeProjectile::OnHit);
	RootComponent = CollisionComp;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	ProjectileMesh->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.8f;

	ExplosionStrategyClass = UC_GrenadeExplode::StaticClass();

	// 부모(AC_ThrowableWeaponBase)의 이펙트 기본 스케일 설정 (기본값 1.0f)
	m_ExplosionEffectScale = 1.0f;
}

void AC_GrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

void AC_GrenadeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
	{
		const FVector ExplosionLocation = GetActorLocation();
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
		}

		// -------------------------------------------------------------
		// 🔴 2. 폭발 범위 디버그 스피어 (부모의 m_ExplosionRadius 사용)
		// -------------------------------------------------------------
		DrawDebugSphere(
			GetWorld(),
			ExplosionLocation,
			m_ExplosionRadius,
			32,
			FColor::Red,
			false,
			2.0f
		);

		// -------------------------------------------------------------
		// 🎯 3. 폭발 반경 내 폰(Pawn) 검사 및 데미지 전달
		// -------------------------------------------------------------
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (GetInstigator())
		{
			QueryParams.AddIgnoredActor(GetInstigator());
		}

		TArray<FOverlapResult> OverlapResults;
		bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
			OverlapResults,
			ExplosionLocation,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(m_ExplosionRadius),
			QueryParams
		);

		if (bHasOverlap)
		{
			TSet<AActor*> DamagedActors;

			for (const FOverlapResult& Result : OverlapResults)
			{
				AActor* Target = Result.GetActor();
				if (!Target || DamagedActors.Contains(Target)) continue;

				// 장애물 검사
				FHitResult BlockHit;
				FVector TraceStart = ExplosionLocation + FVector(0.f, 0.f, 50.f);
				FVector TraceEnd = Target->GetActorLocation() + FVector(0.f, 0.f, 50.f);

				bool bBlocked = GetWorld()->LineTraceSingleByChannel(
					BlockHit,
					TraceStart,
					TraceEnd,
					m_ExplosionTraceChannel, // 부모의 TraceChannel 사용
					QueryParams
				);

				if (!bBlocked || BlockHit.GetActor() == Target)
				{
					DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 2.0f, 0, 1.5f);

					AController* InstigatorController = GetInstigator() ? GetInstigator()->GetController() : nullptr;

					// 거리 비례 데미지 적용
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
				else
				{
					DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.0f, 0, 1.5f);
				}
			}
		}

		// 폭발 후 액터 파괴
		Destroy();
	}
}