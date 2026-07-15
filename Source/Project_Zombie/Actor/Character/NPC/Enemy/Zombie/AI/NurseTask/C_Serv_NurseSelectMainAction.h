// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "C_Serv_NurseSelectMainAction.generated.h"

/**
 * Nurse가 어떤 MainAction을 취해야하는지 결정하는 Service (Interval 보통보다 길게 처리될 예정)
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Serv_NurseSelectMainAction : public UBTService
{
	GENERATED_BODY()
};
