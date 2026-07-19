// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_EnemyProjectile.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_EnemyProjectile : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent*				m_Sphere; // PMC 의 물리 시뮬레이션을 위해서 추가

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UProjectileMovementComponent* m_PMC; // 투사체 움직임 제어 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UNiagaraComponent*			m_NiagaraCom; // 나이아가라 재생 컴포넌트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UNiagaraSystem*				m_ProjectileEffect; // 투사체 시각효과 이펙트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UNiagaraSystem*				m_HitEffect; // 투사체가 히트했을 때 발생할 이펙트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float								m_LifeTime; // 투사체 수명

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class AC_BasicEnemy*				m_SkillUser; // 투사체 생성자

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class UC_EnemySkillData*			m_Skill; // 투사체를 생성시킨 스킬

public:
	void InitProjectile(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// 투사체가 충돌했을 때 호출되는 함수
	/// </summary>
	virtual void OnHit(AActor* _OtherActor, UPrimitiveComponent* _OtherCom, const FHitResult& _Hit);

	/// <summary>
	/// HitEffect를 스폰하는 함수
	/// </summary>
	void SpawnHitEffect(const FVector& _Location);

	/// <summary>
	/// HitSound를 재생하는 함수
	/// </summary>
	void PlayHitSound(const FVector& _Location);


	/// <summary>
	/// 바닥과 충돌할때 호출될 함수
	/// </summary>
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);


public:
	AC_EnemyProjectile();
};
