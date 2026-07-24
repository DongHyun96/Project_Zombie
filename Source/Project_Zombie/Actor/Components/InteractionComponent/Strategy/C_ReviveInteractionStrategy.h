// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Components/InteractionComponent/Strategy/C_InteractionStrategyBase.h"
#include "C_ReviveInteractionStrategy.generated.h"


class AC_BasicPlayer;

UCLASS()
class PROJECT_ZOMBIE_API UC_ReviveInteractionStrategy : public UC_InteractionStrategyBase
{
	GENERATED_BODY()
	
public:
	UC_ReviveInteractionStrategy();

public:

	virtual bool CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const override;

	virtual bool CanContinueInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const override;

	virtual void StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) override;

	virtual void CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) override;

	virtual void CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) override;

private:
	// Revive 시, DownedPlayer의 체력 회복량
	UPROPERTY(EditDefaultsOnly, Category = "Revive")
	float m_ReviveHealth = 10.0f;

};
