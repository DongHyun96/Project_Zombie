// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/C_BasicNPC.h"
#include "C_BasicEnemy.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicEnemy : public AC_BasicNPC
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "StatComponent"))
	class UC_EnemyStatComponent*			m_StatCom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "SkillComponent"))
	class UC_EnemySkillComponent*			m_SkillCom;
	
public:
	AC_BasicEnemy();
};
