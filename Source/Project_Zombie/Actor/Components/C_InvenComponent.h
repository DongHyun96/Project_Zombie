// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "Multi/C_InvenStructures.h"
#include "C_InvenComponent.generated.h"

struct FInventoryContainer;

// C_InventoryGridWidget의 ItemSlot업데이트를 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, int32, SlotIndex, const FInventoryEntry&, ItemData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_InvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UC_InvenComponent();

public:
    const TArray<FInventoryEntry>& GetInventoryItems() const { return InventoryContainer.Items; }
	
	// 특정 슬롯의 아이템 반환
	const FInventoryEntry& GetItemAt(int32 SlotIndex) const {return InventoryContainer.Items[SlotIndex];}

	// ItemLinkComponent 등에서 원본 데이터를 직접 가리키기 위한 포인터 반환 함수
	FInventoryEntry* GetSlotDataPtr(int32 SlotIndex)
    {
    	if (InventoryContainer.Items.IsValidIndex(SlotIndex))
    	{
    		return &InventoryContainer.Items[SlotIndex];
    	}
    	return nullptr;
    }
	
	bool GetHasEquipmentSlots() const {return bHasEquipmentSlots;}

	// 원본 포인터 데이터를 다 수정한 후 FastArray 복제 상태를 갱신해 주는 함수
	void MarkSlotDirty(int32 SlotIndex)
    {
    	if (InventoryContainer.Items.IsValidIndex(SlotIndex))
    	{
    		InventoryContainer.MarkItemDirty(InventoryContainer.Items[SlotIndex]);
    	}
    }
	
	int32 GetTotalItemCount(const FName& InItemRowName);
	
	// TODO : 현재 사용하지 않음
	void SetContainerID(int32 _ContainerID) { ContainerID = _ContainerID; }
	
	// TODO : 현재 사용하지 않음
	int32 GetContainerID() { return ContainerID; }
	
public:
	// //InvenComponent의 InventoryContainer의 TArray<FInventoryEntry>의 크기를 InMax 초기화 해주고 업데이트 해주는 함수.
	void SetMaxSlots(int32 InMax)
	{
		MaxSlots = InMax;
		
		InitInventoryContainerMaxSlots(MaxSlots);
	}

	void SetEntryCurCount(int32 Idx, int32 InCount);
	
	void SetHasEquipmentSlots(bool bIsEquip) { bHasEquipmentSlots = bIsEquip; }
	
	// 특정 슬롯의 아이템 초기화
	void InitInvenItemAt(int32 Idx);
	
	UFUNCTION(BlueprintCallable)
	int32 AddItem(FInventoryEntry ItemEntry);
	
	// 아이템의 Type에 따라 장비인덱스값에는 들어갈 수 있는지 판단하는 함수. 
	bool CanSetItemToSlot(int32 TargetSlotIndex, const FInventoryEntry& Entry) const;
public:
	// 아이템이 드래그 드롭 되었을 때 처리해주는 함수.
	void ProcessItemMove(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 InPlayerID);
	
	// 드래그 시작
	void StartDragItemSlot(int32 SlotIndex, int32 InPlayerId);
	
	void CancelDragItemSlot(int32 SlotIndex, int32 InPlayerId);
	
	// 인벤토리 강제 동기화
	UFUNCTION(BlueprintCallable)
	void ForceRepInven();

	// TODO : 팅김시 드래그 락 해결 테스트 함수
	void ReleaseAllLocksByPlayer(int32 InPlayerID);
	
	// TODO : 팅김시 드래그 락 해결 테스트 함수
	void ForceReleaseSlotLock(int32 SlotIndex);
	
	// TODO : 이렇게 서버에서 Player - Inven의 함수 호출 하는 경우에는 델리게이트를 사용하는게 좋나? 
	bool ProcessItemDivideMove(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 SplitCount, int32 InPlayerID);
	
	// 외부(Player)에서 안전하게 호출할 인벤토리 내 아이템 분할 차감 함수
	// 성공 시 실제로 필드에 드롭해야 할 수량(SplitCount)을 반환하고, 실패 시 0을 반환합니다.
	int32 ProcessItemDivideDrop(int32 SrcIdx, int32 SplitCount, int32 InPlayerID);
	
	// 슬롯에 명시적으로 잠금을 걸거나 해제하는 함수 (서버 권한 필요)
	UFUNCTION(BlueprintCallable)
	bool SetSlotLockState(int32 SlotIdx, int32 InPlayerID);
	
protected:
	virtual void BeginPlay() override;
    
    /// <summary>
    /// 언리얼 엔진의 네트워크 시스템이 액터나 컴포넌트가 처음 생성될 때 그리고 네트워크 상에 등록될 때 
    /// 엔진 내부에서 자동으로 호출해주는 함수.
    /// </summary>
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 아이템 위치 스위칭
	bool SwapInvenEntry(int32 MySlotIdx, UC_InvenComponent* TargetComp, int32 TargetSlotIdx, int32 InPlayerID);
	
	// 아이템 병합
	bool TryMergeItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 InPlayerID, int32 MaxCount);
	
	// InventoryContainer가 가지고 있는 TArray<FInventoryEntry>의 크기를 초기화 해주는 함수.
	void InitInventoryContainerMaxSlots(int32 InMax);
	
	UFUNCTION(BlueprintCallable)
	void OnRep_InventoryContainer();

protected:
    // C_IneventoryGridWidget에서 grid slot의 갯수와 일치 시킬 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxSlots = 45; 
	
	// Fast Array Container
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UPROPERTY(ReplicatedUsing = OnRep_InventoryContainer,EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInventoryContainer InventoryContainer;
	
	// TODO : 현재 미사용 중, 현재 포인터를 패킷으로 보내고 있는데(엔진에서 GUID로 변환해줌.)
	//		  이걸 사용하면 int32를 패킷으로 보내서 서버에서 해당 id의 객체를 찾아 접근 할 수 있음
	// 고유 ID 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ContainerID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bHasEquipmentSlots = false;
public:
    UPROPERTY(BlueprintAssignable)
    FOnInventorySlotChanged OnInventorySlotChanged; // 델리게이트 알림용 변수
};
