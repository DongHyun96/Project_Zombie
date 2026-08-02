// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Interact/C_InteractableBase.h"
#include "C_ItemUpgradeStation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_ItemUpgradeStation : public AC_InteractableBase
{
	GENERATED_BODY()

public:
	AC_ItemUpgradeStation();
	
	void RequestItemUpgrade(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat);
protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UC_ItemUpgradeComponent* m_UpgradeComp{};
};
