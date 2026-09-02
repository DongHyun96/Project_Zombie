// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Components/InteractionComponent/Strategy/C_InteractionStrategyBase.h"
#include "C_SkinWidgetStrategy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_SkinWidgetStrategy : public UC_InteractionStrategyBase
{
	GENERATED_BODY()

public:
	UC_SkinWidgetStrategy();
	
public:

	virtual bool CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const override;

	virtual bool StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) override;

	virtual void CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) override;

	virtual void CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) override;

};
