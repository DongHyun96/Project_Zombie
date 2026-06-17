// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMetaData.h"
#include "C_MontagePriorityMetaData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_MontagePriorityMetaData : public UAnimMetaData
{
	GENERATED_BODY()

public:
	
	const FGameplayTag& GetMontagePriorityTag() const { return m_MontagePriorityTag; }
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority")
	FGameplayTag m_MontagePriorityTag{};
	
};
