// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Components/ActorComponent.h"
#include "C_StatUpgradeComponent.generated.h"


class UC_InvenComponent;
class AC_BasicPlayer;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_StatUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UC_StatUpgradeComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void UpgradeStat(AC_BasicPlayer* InPlayer, const FName& UpStatName);
	
private:
	// 스탯 업그레이드 재료 소모 처리
	void ConsumeUpgradeMaterials(UC_InvenComponent* InvenComp, const FGradeCostInfo& CostInfo);

	// 업그레이드 진행 상태 종료 처리 (Local / Client 구분)
	void FinishUpgradeState(AC_BasicPlayer* InPlayer);
};
