// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_GunDataTableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_GunDataTableComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FName				m_RowName;

	// 보유 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TMap<FName, float>	m_Stats;

public:	
	UC_GunDataTableComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
