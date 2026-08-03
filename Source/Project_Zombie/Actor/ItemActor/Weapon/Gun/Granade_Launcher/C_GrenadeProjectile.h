// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_GrenadeProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UC_GrenadeExplode;

UCLASS()
class PROJECT_ZOMBIE_API AC_GrenadeProjectile : public AActor
{
	GENERATED_BODY()

private:
	// 충돌체 (Root)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComp;

	// 유탄 외형 메시
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	// 투사체 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 최대 데미지
	UPROPERTY(VisibleAnywhere, Category = "Explosion")
	float m_MaxDamage;

	// 최소 데미지
	UPROPERTY(VisibleAnywhere, Category = "Explosion")
	float m_MinDamage;

	// 폭발 전략 클래스
	UPROPERTY(VisibleAnywhere, Category = "Explosion")
	TSubclassOf<UC_GrenadeExplode> ExplosionStrategyClass;

	// 폭발 반경
	UPROPERTY(VisibleAnywhere, Category = "Explosion")
	float m_ExplosionRadius;

	// 폭발 이펙트 
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	TObjectPtr<UParticleSystem> m_ExplosionEffect{};
	
	// 폭발 이펙트 크기 (1.0 = 기본 크기)
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	float m_ExplosionEffectScale;

	bool m_bHasExploded = false;

public:
	UProjectileMovementComponent* GetProjectileMovement() { return ProjectileMovement; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayExplosionFX(FVector ExplosionLocation);

public:
	AC_GrenadeProjectile();

};