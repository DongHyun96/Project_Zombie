// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/C_WeaponDataTableComponent.h"
#include "C_GunDataTableComponent.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_GunDataTableComponent : public UC_WeaponDataTableComponent
{
	GENERATED_BODY()
protected:
	// 데이터 테이블 포인터
	UPROPERTY(EditAnywhere, Category = "WeaponData", meta = (RequiredAssetDataTags = "RowStructure=/Script/Project_Zombie.GunData"))
	UDataTable* m_Table;

public:
	virtual void BeginPlay() override;
	virtual void InitData() override;

public:
	UC_GunDataTableComponent();
};
