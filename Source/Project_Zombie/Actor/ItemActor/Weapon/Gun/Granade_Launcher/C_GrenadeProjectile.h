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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float m_MaxDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float m_MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float m_ExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UParticleSystem* m_ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float m_ExplosionEffectScale;

	bool m_bHasExploded = false;

	bool m_bHasToExplodeOnSpawn{}; // 총구가 Muzzle Awareness 거리에 들어간 경우, 바로 해당 위치에서 폭파시켜버림
	
protected:
	virtual void BeginPlay() override;

	class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestExplode(FVector ExplosionLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayExplosionFX(FVector ExplosionLocation);

	void ExplodeInternal(FVector ExplosionLocation);

private:
	
	/// <summary>
	/// 폭파 시작 처리
	/// </summary>
	void OnExplodeStart();
	
public:
	
	void SetHasToExplodeOnSpawn() { m_bHasToExplodeOnSpawn = true; }
	
public:
	AC_GrenadeProjectile();
};