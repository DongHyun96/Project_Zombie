// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/ThrowableWeapon/C_ThrowableWeaponBase.h"
#include "C_GrenadeProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UC_GrenadeExplode;

UCLASS()
class PROJECT_ZOMBIE_API AC_GrenadeProjectile : public AC_ThrowableWeaponBase
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

	// 폭발 전략 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	TSubclassOf<UC_GrenadeExplode> ExplosionStrategyClass;

public:
	UProjectileMovementComponent* GetProjectileMovement() { return ProjectileMovement; }

protected:
	virtual void BeginPlay() override;

	// 충돌 시 호출될 콜백 함수
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	AC_GrenadeProjectile();

};