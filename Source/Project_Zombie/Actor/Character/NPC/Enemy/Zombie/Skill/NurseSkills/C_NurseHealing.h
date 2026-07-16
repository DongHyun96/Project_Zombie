// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillBase.h"
#include "C_NurseHealing.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_NurseHealing : public UC_EnemySkillBase
{
	GENERATED_BODY()
	
	virtual bool Activate(AC_BasicEnemy* _Owner,UC_EnemySkillData* _Data) override;
	
	virtual void Fire(AC_BasicEnemy* _Owner,UC_EnemySkillData* _Data) override;
};
