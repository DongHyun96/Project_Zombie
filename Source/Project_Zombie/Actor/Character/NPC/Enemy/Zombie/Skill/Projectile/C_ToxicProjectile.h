// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/Projectile/C_EnemyProjectile.h"
#include "C_ToxicProjectile.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_ToxicProjectile : public AC_EnemyProjectile
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AC_ToxicPool> m_ToxicPoolClass; // 투사체가 충돌했을 때 생성될 독장판 클래스

	bool m_bFinished = false; // ReachTarget과 OnHit이 같이 호출되지 않게하기위한 변수
	
	// projectilemovementcomponent 관련
	UPROPERTY(EditDefaultsOnly, Category = "ToxicProjectile")
	float m_GravityScale = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "ToxicProjectile")
	bool m_bUseHighArc = false;

	UPROPERTY(EditDefaultsOnly, Category = "ToxicProjectile")
	float m_ArcVelocityOverride = 0.f;

protected:
	void OnHit(AActor* _OtherActor, UPrimitiveComponent* _OtherCom, const FHitResult& _Hit) override;

public:
	virtual void InitProjectile(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill, const FVector& _TargetLocation) override;

private:
	/// <summary>
	/// 독장판 생성시 공통 함수 
	/// </summary>
	void SpawnPoolAtGround(const FVector& _ImpactLocation, AActor* _IgnoreActor);

	/// <summary>
	/// 바닥을 추적하는 함수
	/// </summary>
	bool FindPoolGround(const FVector& _ImpactLocation, AActor* _IgnoreActor, FVector& _OutSpawnLocation, FRotator& _OutSpawnRotation) const;

	/// <summary>
	/// 독장판 생성 함수
	/// </summary>
	void SpawnToxicPool(const FVector& _SpawnLocation, const FRotator& _SpawnRotation);

public:
	AC_ToxicProjectile();
};
