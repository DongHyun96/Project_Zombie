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
	TSubclassOf<class AC_ToxicPool> m_ToxicPoolClass; // 투사체가 충돌했을 때 생성될 독성 풀 클래스
	
public:
	void OnHit() override;

public:
	AC_ToxicProjectile();
};
