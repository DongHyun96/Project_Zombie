// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/C_EnemySkillBase.h"
#include "C_NormalAttack.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_NormalAttack : public UC_EnemySkillBase
{
	GENERATED_BODY()

public:
	virtual void Activate(class AC_BasicEnemy* _Owner, class UC_EnemySkillData* _Data) override;

public:
	UC_NormalAttack();

	
};
