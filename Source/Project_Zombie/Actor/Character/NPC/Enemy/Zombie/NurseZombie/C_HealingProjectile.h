// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_HealingProjectile.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_HealingProjectile : public AActor
{
	GENERATED_BODY()

public:
	
	AC_HealingProjectile();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	bool Fire
	(
		const FVector& 		  _FireStartLocation,
		const FVector& 		  _FireDirection,
		class AC_NurseZombie* _SpawnedBy,
		class AC_BasicEnemy*  _HealingTarget,
		float				  _TotalHealAmount
	);

private:
	
	/// <summary>
	/// 비활성화 처리
	/// </summary>
	void Deactivate();

	UFUNCTION()
	void OnHealTargetDead(class AC_BasicCharacter* _DeadCharacter);
	
public:
	
	bool IsActive() const { return m_bActive; }
	
private:

	UFUNCTION()
	void OnMainColliderBeginOverlap
	(
		UPrimitiveComponent* _OverlapComponent,
		AActor*				 _OtherActor,
		UPrimitiveComponent* _OtherComp,
		int32				 _OtherBodyIndex,
		bool				 _bFromSweep,
		const FHitResult&	 _SweepResult
	);
	
	UFUNCTION()
	void OnMainColliderHit
	(
		UPrimitiveComponent* _HittedComponent,
		AActor*				 _OtherActor,
		UPrimitiveComponent* _OtherComp,
		FVector				 _NormalImpulse,
		const FHitResult&	 _HitResult
	);
	
	
private:

	bool			m_bActive{};
	FVector			m_FireDirection{};

	UPROPERTY()
	AC_NurseZombie* m_SpawnedBy{}; // 이 Projectile을 Spawn시킨 NurseZombie 

	UPROPERTY()
	AC_BasicEnemy* m_HealTarget{}; // Homing ProjMovement의 Target
	
	float			m_TotalHealAmount{}; // 힐 주는 량
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float m_Speed{};
	
protected:
	
	UPROPERTY(EditDefaultsOnly, DisplayName = "MainCollider")
	class USphereComponent* m_MainCollider{};

	UPROPERTY(EditDefaultsOnly, Category = "Components", DisplayName = "ProjectileMovementCom")
	class UProjectileMovementComponent* m_ProjectileMovement{};

protected: /* Effect 관련 */
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", DisplayName = "NiagaraComponent")
	class UNiagaraComponent* m_NiagaraComponent{}; // 나이아가라 재생 컴포넌트

	// Projectile 뒤따라 지속적으로 보일 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", DisplayName = "TrailEffect")
	class UNiagaraSystem* m_TrailEffect{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", DisplayName = "HealEffect")
	UNiagaraSystem* m_HealEffect{}; // 투사체가 대상에 제대로 충돌하여 Heal을 줄 때 이펙트
	
};
