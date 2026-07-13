// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "C_ZombieManager.generated.h"

enum class EZombieType : uint8;
/**
 * Zombie 스폰 및 Zombie 객체 Holder
 */
UCLASS(Blueprintable)
class PROJECT_ZOMBIE_API UC_ZombieManager : public UObject
{
	GENERATED_BODY()

public:
	
	UC_ZombieManager();

	void OnWorldBeginPlay();
	
protected:

	// 스폰시킬 좀비 클래스들
	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, TSubclassOf<class AC_Zombie>> m_ZombieClasses{};

	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, uint32> m_PoolCounts{};

protected:
	
	
	
};
