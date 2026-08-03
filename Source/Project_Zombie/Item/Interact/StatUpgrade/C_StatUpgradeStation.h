// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Interact/C_InteractableBase.h"
#include "C_StatUpgradeStation.generated.h"

class UC_StatUpgradeComponent;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_StatUpgradeStation : public AC_InteractableBase
{
	GENERATED_BODY()
	
public:
	AC_StatUpgradeStation();
	
	// void RequestStatUpgrade(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat);
protected:
	virtual void BeginPlay() override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UC_StatUpgradeComponent* m_UpgradeComp{};
};
