// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "C_ItemUpgradeComponent.generated.h"

class AC_BasicPlayer;

// 아이템을 강화하는 컴포넌트
// InteractableBase쪽에 붙어서 UpgradeWidget을 통해 클라가 서버에 강화를 요청하도록 할 예정.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_ItemUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UC_ItemUpgradeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void UpgradeItem(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat);
};
