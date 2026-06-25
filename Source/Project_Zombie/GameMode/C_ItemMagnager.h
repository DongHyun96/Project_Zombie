// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GlobalData.h"
#include "C_ItemMagnager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ItemMagnager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FName 키값으로 아이템 데이터 포인터를 빠르게 반환하는 함수
	const FItemData* GetItemData(FName InRowName) const;

private:
	UPROPERTY()
	UDataTable* ItemDataTable = nullptr;
};
