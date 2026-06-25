// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "C_InvenComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_InvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UC_InvenComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 이 함수는 현재 사용하지 않을 예정
    /// <summary>
    /// 언리얼 엔진의 네트워크 시스템이 액터나 컴포넌트가 처음 생성될 때 그리고 네트워크 상에 등록될 때 
    /// 엔진 내부에서 자동으로 호출해주는 함수.
    /// </summary>
    //virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventoryEntry> InventoryItems;
};
