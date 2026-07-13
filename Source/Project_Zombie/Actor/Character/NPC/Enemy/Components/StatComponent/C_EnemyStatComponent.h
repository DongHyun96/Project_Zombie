// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "C_EnemyStatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EnemyStatComponent : public UC_StatComponentBase
{
	GENERATED_BODY()
	

protected:
	
	// virtual void InitAdditionalStat() override {}
	
public:
	
	UC_EnemyStatComponent();

	virtual void BeginPlay() override;
	
private:
	
	virtual UScriptStruct* GetStatDataStruct() const override;
};
