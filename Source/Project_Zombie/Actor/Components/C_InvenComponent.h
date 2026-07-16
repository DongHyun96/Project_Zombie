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

	void SetContainerID(int32 _ContainerID) { ContainerID = _ContainerID; }
	
	int32 GetContainerID() { return ContainerID; }
	
public:
	// 아이템이 드래그 드롭 되었을 때 처리해주는 함수.
	void ProcessItemMove(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 InPlayerID);

	void SetEntryCurCount(int32 Idx, int32 InCount);
	
	// 특정 슬롯의 아이템 초기화
	void InitInvenItemAt(int32 Idx);
	
	UFUNCTION(BlueprintCallable)
	int32 AddItem(FInventoryEntry ItemEntry);
	
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
	
protected:
	virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetItemLock(int32 SlotIdx, int32 InPlayerID = INDEX_NONE)
	{
		InventoryContainer.Items[SlotIdx].LockedByPlayerID = INDEX_NONE;
	}
    
    /// <summary>
    /// 언리얼 엔진의 네트워크 시스템이 액터나 컴포넌트가 처음 생성될 때 그리고 네트워크 상에 등록될 때 
    /// 엔진 내부에서 자동으로 호출해주는 함수.
    /// </summary>
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 아이템 위치 스위칭
	bool SwapInvenEntry(int32 MySlotIdx, UC_InvenComponent* TargetComp, int32 TargetSlotIdx, int32 InPlayerID);
	
	// 아이템 병합
	bool TryMergeItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 InPlayerID, int32 MaxCount);
	
	UFUNCTION(BlueprintCallable)
	void OnRep_InventoryContainer();
	
	// 커서 슬롯 변경 시 호출할 함수
	//UFUNCTION()
	//void OnRep_CursorSlot();
protected:
    // C_IneventoryGridWidget에서 grid slot의 갯수와 일치 시킬 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxSlots = 45; 
	
	// Fast Array Container
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UPROPERTY(ReplicatedUsing = OnRep_InventoryContainer,EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInventoryContainer InventoryContainer;

	//UPROPERTY(Replicated)
	//FCusorItem CursorSlot;
	
	// TODO : 현재 미사용 중, 현재 포인터를 패킷으로 보내고 있는데(엔진에서 GUID로 변환해줌.)
	//		  이걸 사용하면 int32를 패킷으로 보내서 서버에서 해당 id의 객체를 찾아 접근 할 수 있음
	// 고유 ID 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ContainerID;
public:
    UPROPERTY(BlueprintAssignable)
    FOnInventorySlotChanged OnInventorySlotChanged; // 델리게이트 알림용 변수
};
