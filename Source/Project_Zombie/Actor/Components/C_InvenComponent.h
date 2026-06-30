// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "C_InvenComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, int32, SlotIndex, const FInventoryEntry&, ItemData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_InvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UC_InvenComponent();

public:
    const TArray<FInventoryEntry>& GetInventoryItems() const { return InventoryItems; }

    bool SwapInvenEntry(int32 SlotIdx1, int32 SlotIdx2);
	
	void InitInvenItemAt(int32 idx);

protected:
	virtual void BeginPlay() override;

public:	
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 이 함수는 현재 사용하지 않을 예정
    /// <summary>
    /// 언리얼 엔진의 네트워크 시스템이 액터나 컴포넌트가 처음 생성될 때 그리고 네트워크 상에 등록될 때 
    /// 엔진 내부에서 자동으로 호출해주는 함수.
    /// </summary>
    //virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool AddItem(FInventoryEntry ItemEntry);
protected:
    // C_IneventoryGridWidget에서 grid slot의 갯수와 일치 시킬 변수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxSlots = 45; 

    // 인벤토리에서 담고 있을 아이템 정보 구조체 배열
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventoryEntry> InventoryItems;

public:
    UPROPERTY(BlueprintAssignable)
    FOnInventorySlotChanged OnInventorySlotChanged; // 델리게이트 알림용 변수
};
