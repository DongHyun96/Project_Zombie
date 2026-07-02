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
	
	// 특정 슬롯의 아이템 반환
	const FInventoryEntry& GetItemAt(int32 SlotIndex) const {return InventoryItems[SlotIndex];}

	// 아이템 위치 스위칭
    bool SwapInvenEntry(int32 SlotIdx1, int32 SlotIdx2);

	// 특정 슬롯의 아이템 초기화
	void InitInvenItemAt(int32 idx);

	// 다른 인벤(창고)와 아이템을 교환 / 이동할 때 사용하는 함수.
	bool TransferItemTo(int32 MySlotIdx, UC_InvenComponent* TargetComp, int32 TargetSlotIdx);

	UFUNCTION(BlueprintCallable)
	bool AddItem(FInventoryEntry ItemEntry);
	
	// 클라이언트가 서버로 아이템 이동을 요청하는 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestMoveItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx);
protected:
	virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 이 함수는 현재 사용하지 않을 예정
    /// <summary>
    /// 언리얼 엔진의 네트워크 시스템이 액터나 컴포넌트가 처음 생성될 때 그리고 네트워크 상에 등록될 때 
    /// 엔진 내부에서 자동으로 호출해주는 함수.
    /// </summary>
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버에서 데이터가 바뀌었을 때 클라이언트에서 호출될 OnRep 함수
	UFUNCTION()
	void OnRep_InventoryItems();
protected:
    // C_IneventoryGridWidget에서 grid slot의 갯수와 일치 시킬 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxSlots = 45; 

    // 인벤토리에서 담고 있을 아이템 정보 구조체 배열
	UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventoryEntry> InventoryItems;

public:
    UPROPERTY(BlueprintAssignable)
    FOnInventorySlotChanged OnInventorySlotChanged; // 델리게이트 알림용 변수
};
