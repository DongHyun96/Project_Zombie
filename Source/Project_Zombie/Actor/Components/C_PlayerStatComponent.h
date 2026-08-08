// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatComponent/C_StatComponentBase.h"
#include "C_PlayerStatComponent.generated.h"


class AC_StatUpgradeStation;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PlayerStatComponent : public UC_StatComponentBase
{
	GENERATED_BODY()

public:
	
	UC_PlayerStatComponent();

public:
	
	virtual void BeginPlay() override;
	
	void Server_RequestStatUpgrade(AC_StatUpgradeStation* InInteractableActor, const FName& UpStatName);
private:
	
	virtual UScriptStruct* GetStatDataStruct() const override;

private:
	
	void UpdateOtherPlayerHPBar(float _Ratio);
	
private:

	UPROPERTY()
	class AC_BasicPlayer* m_OwnerPlayer{};
	
	UPROPERTY()
	class UC_GameMainHUD* m_MainHUD{};
	
};
