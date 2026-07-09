// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillBase.h"
#include "C_PoisonAttack.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_PoisonAttack : public UC_EnemySkillBase
{
	GENERATED_BODY()
	
public:
	/// <summary>
	/// 스킬 구현함수
	/// </summary>
	virtual void Activate(class AC_BasicEnemy* _Owner, class UC_EnemySkillData* _Data) override;

public:
	UC_PoisonAttack();
};
