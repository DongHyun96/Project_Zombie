// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatComponent/C_StatComponentBase.h"
#include "C_PlayerStatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PlayerStatComponent : public UC_StatComponentBase
{
	GENERATED_BODY()

public:
	
	UC_PlayerStatComponent();
	
private:
	
	virtual UScriptStruct* GetStatDataStruct() const override;

};
