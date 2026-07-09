// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../C_BasicEnemy.h"
#include "C_Zombie.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_Zombie : public AC_BasicEnemy
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UC_EnemySkillComponent* GetSkillComponent() { return m_SkillCom; }

public:	
	virtual void Tick(float DeltaTime) override;

	AC_Zombie();
};
