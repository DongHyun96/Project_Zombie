#include "Actor/Components/C_InvenComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Utility/C_Util.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "Net/UnrealNetwork.h"
UC_InvenComponent::UC_InvenComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
	
	//InventoryContainer.Items.Init(FInventoryEntry(), MaxSlots);
	// 컴포넌트 리플리케이션 활성화. 
	SetIsReplicatedByDefault(true);
	
	//if (GetOwner())
	//{
	//	ContainerID = GetOwner()->GetUniqueID();
	//	이러면 actor의 id를 받아오는것이라 따로 고유 번호를 주는 방법을 사용하면 좋을 듯?
	//}
}

void UC_InvenComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryContainer.OwnerComponent = this;
	
	if (GetOwner()->HasAuthority())
	{
		// 서버는 무조건 45개로 초기화 (기존 데이터가 있다면 덮어씀)
		InventoryContainer.Items.SetNum(MaxSlots);
       
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			// 데이터가 비어있을 때만 Clear, 이미 데이터가 있다면(세이브 로드 등) 건드리지 않음
			if (InventoryContainer.Items[i].ItemRowName == NAME_None)
			{
				InventoryContainer.Items[i].Clear();
			}
			InventoryContainer.Items[i].SlotIndex = i;
		}
		
		//FInventoryEntry InventoryEntry{};
		//InventoryEntry.ItemRowName = FName("Item_Zombium");
		//InventoryEntry.Count = 1;
		//AddItem(InventoryEntry);
		InventoryContainer.MarkArrayDirty();
	}
}

void UC_InvenComponent::ProcessItemMove(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp,
	int32 DstIdx, int32 InPlayerID)
{
	FInventoryEntry& SrcEntry = SrcComp->InventoryContainer.Items[SrcIdx];
	FInventoryEntry& DstEntry = DstComp->InventoryContainer.Items[DstIdx];
    
	// 병합 가능한 상황인지 확인
	bool bIsSameItem = (SrcEntry.ItemRowName != NAME_None && SrcEntry.ItemRowName == DstEntry.ItemRowName);

	if (bIsSameItem)
	{
		if (!GetWorld()) return;
		
		UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
		
		if (!ItemManager) return;
		
		int32 MaxCount = ItemManager->GetItemData(SrcEntry.ItemRowName)->MaxCount;
           
		if (MaxCount > 1)
		{
			// 병합에 성공하면 조기 종료
			if (!TryMergeItem(SrcComp, SrcIdx, DstComp, DstIdx, InPlayerID, MaxCount)) return;
		}
	}

	// 병합을 할 수 없거나(다른 아이템, 꽉 참), 스택 불가 아이템인 경우 -> 무조건 스왑/이동
	SrcComp->SwapInvenEntry(SrcIdx, DstComp, DstIdx, InPlayerID);
}

void UC_InvenComponent::SetEntryCurCount(int32 Idx, int32 InCount)
{
	InventoryContainer.Items[Idx].CurCount = InCount;
}


void UC_InvenComponent::InitInvenItemAt(int32 Idx)
{
	InventoryContainer.Items[Idx].Clear();
}

int32 UC_InvenComponent::AddItem(FInventoryEntry ItemEntry)
{
	if (ItemEntry.ItemRowName == NAME_None || ItemEntry.CurCount <= 0) return ItemEntry.CurCount;

	if (!GetWorld()) return ItemEntry.CurCount;
	
    UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
    
	if (!ItemManager) return ItemEntry.CurCount;

    int32 MaxCount = ItemManager->GetItemData(ItemEntry.ItemRowName)->MaxCount;
    int32 RemainCount = ItemEntry.CurCount; // 넣어야 할 남은 수량

    // 1. 기존에 존재하는 동일한 아이템 슬롯 찾아서 채워 넣기 (스택 가능 아이템)
    if (MaxCount > 1)
    {
        for (int32 i = 0; i < MaxSlots; ++i)
        {
            if (InventoryContainer.Items[i].ItemRowName == ItemEntry.ItemRowName)
            {
                int32 SpaceLeft = MaxCount - InventoryContainer.Items[i].CurCount; // 슬롯의 남은 공간
                
                if (SpaceLeft > 0)
                {
                    // 남은 수량과 남은 공간 중 더 작은 값을 더함
                    int32 AddAmount = FMath::Min(RemainCount, SpaceLeft);
                    
                    InventoryContainer.Items[i].CurCount += AddAmount;
                    RemainCount -= AddAmount;

                    // 동기화
                    InventoryContainer.MarkItemDirty(InventoryContainer.Items[i]);
                    if (GetOwner()->HasAuthority())
                    {
                        OnInventorySlotChanged.Broadcast(i, InventoryContainer.Items[i]);
                    }

                    // 다 넣었으면 0 반환하고 즉시 종료
                    if (RemainCount <= 0) return 0; 
                }
            }
        }
    }

    // 2. 아직 남은 수량이 있다면, 빈 슬롯을 찾아 순차적으로 채워 넣기
    for (int32 i = 0; i < MaxSlots; ++i)
    {
        if (InventoryContainer.Items[i].ItemRowName == NAME_None)
        {
            // 빈 슬롯에는 MaxCount만큼 넣거나, 남은 수량만큼 넣거나
            int32 AddAmount = FMath::Min(RemainCount, MaxCount);

            // 데이터 덮어쓰기 (SlotIndex와 잠금 상태 유지)
            InventoryContainer.Items[i] = ItemEntry; 
            InventoryContainer.Items[i].CurCount = AddAmount;
            InventoryContainer.Items[i].SlotIndex = i; 
            InventoryContainer.Items[i].LockedByPlayerID = INDEX_NONE;

            RemainCount -= AddAmount;

            // 동기화
            InventoryContainer.MarkItemDirty(InventoryContainer.Items[i]);
            if (GetOwner()->HasAuthority())
            {
                OnInventorySlotChanged.Broadcast(i, InventoryContainer.Items[i]);
            }

            // 다 넣었으면 0 반환하고 즉시 종료
            if (RemainCount <= 0) return 0; 
        }
    }

    // 3. 인벤토리가 가득 차서 다 넣지 못한 경우 남은 수량 반환
    return RemainCount;
}

void UC_InvenComponent::ForceRepInven()
{
	InventoryContainer.MarkArrayDirty();
}

void UC_InvenComponent::ReleaseAllLocksByPlayer(int32 InPlayerID)
{
	if (InPlayerID == INDEX_NONE) 
	{
		UE_LOG(LogTemp, Error, TEXT("ReleaseAllLocksByPlayer: Failed! InPlayerID is INDEX_NONE(-1)"));
		return;
	}

	const int32 NumItems = InventoryContainer.Items.Num();
	for (int32 i = 0; i < NumItems; ++i)
	{
		FInventoryEntry& Entry = InventoryContainer.Items[i];

		// 락이 걸려있는 슬롯이 있다면 전부 로그 출력
		if (Entry.LockedByPlayerID != INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Inven [%s] Slot [%d] is currently locked by PlayerID [%d]. (Target Leaver ID: [%d])"), 
				*GetName(), 
				Entry.SlotIndex, 
				Entry.LockedByPlayerID, 
				InPlayerID);
		}

		// 일치하면 잠금 해제
		if (Entry.LockedByPlayerID == InPlayerID)
		{
			CancelDragItemSlot(Entry.SlotIndex, InPlayerID);
            
			UE_LOG(LogTemp, Log, TEXT("Inven [%s] Slot [%d] Lock Released Successfully for Player [%d]!"), 
				*GetName(), 
				Entry.SlotIndex, 
				InPlayerID);
		}
	}
}

void UC_InvenComponent::ForceReleaseSlotLock(int32 SlotIndex)
{
	if (!InventoryContainer.Items.IsValidIndex(SlotIndex)) return;

	InventoryContainer.Items[SlotIndex].LockedByPlayerID = INDEX_NONE;
	InventoryContainer.MarkItemDirty(InventoryContainer.Items[SlotIndex]);

	if (GetOwner()->HasAuthority())
	{
		OnInventorySlotChanged.Broadcast(SlotIndex, InventoryContainer.Items[SlotIndex]);
	}
}

void UC_InvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [최적화] 방장의 부담을 줄이기 위해, 해당 인벤토리의 소유자(클라이언트)에게만 복제
	//DOREPLIFETIME_CONDITION(UC_InvenComponent, InventoryContainer, COND_OwnerOnly);
	
	DOREPLIFETIME(UC_InvenComponent, InventoryContainer);
}

bool UC_InvenComponent::SwapInvenEntry(int32 MySlotIdx, UC_InvenComponent* TargetComp, int32 TargetSlotIdx, int32 InPlayerID)
{
	// 1. 유효성 및 권한 검사
	if (!GetOwner()->HasAuthority() || !TargetComp) return false;
	
	//if (!InventoryContainer.Items.IsValidIndex(MySlotIdx) || !TargetComp->InventoryContainer.Items.IsValidIndex(TargetSlotIdx)) return false;

	FInventoryEntry& MyEntry = InventoryContainer.Items[MySlotIdx];
	FInventoryEntry& TargetEntry = TargetComp->InventoryContainer.Items[TargetSlotIdx];

	// 2. 잠금 검증
	if (MyEntry.LockedByPlayerID != InPlayerID) return false;
	if (TargetEntry.ItemRowName != NAME_None && TargetEntry.LockedByPlayerID != INDEX_NONE) return false;

	// 3. 데이터 백업
	FName TempRowName = MyEntry.ItemRowName;
	int32 TempCount = MyEntry.CurCount;
	int32 TempUpgrade = MyEntry.UpgradeLevel;
	int32 TempAmmo = MyEntry.CurAmmo;

	// 4. 내 가방 <- 타겟 데이터 복사
	MyEntry.ItemRowName = TargetEntry.ItemRowName;
	MyEntry.CurCount = TargetEntry.CurCount;
	MyEntry.UpgradeLevel = TargetEntry.UpgradeLevel;
	MyEntry.CurAmmo = TargetEntry.CurAmmo;

	// 5. 타겟 <- 백업 데이터 복사
	TargetEntry.ItemRowName = TempRowName;
	TargetEntry.CurCount = TempCount;
	TargetEntry.UpgradeLevel = TempUpgrade;
	TargetEntry.CurAmmo = TempAmmo;

	// 6. 잠금 해제
	MyEntry.LockedByPlayerID = INDEX_NONE;
	TargetEntry.LockedByPlayerID = INDEX_NONE;

	// 7. 데이터 갱신 및 브로드캐스트 (TargetComp가 자기 자신이어도 완벽하게 작동함)
	InventoryContainer.MarkItemDirty(MyEntry);
	TargetComp->InventoryContainer.MarkItemDirty(TargetEntry);

	OnInventorySlotChanged.Broadcast(MySlotIdx, MyEntry);
	TargetComp->OnInventorySlotChanged.Broadcast(TargetSlotIdx, TargetEntry);

	return true;
}

bool UC_InvenComponent::TryMergeItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 InPlayerID, int32 MaxCount)
{
	if (!GetOwner()->HasAuthority()) return false;

	FInventoryEntry& SrcEntry = SrcComp->InventoryContainer.Items[SrcIdx];
	FInventoryEntry& DstEntry = DstComp->InventoryContainer.Items[DstIdx];

	// 잠금 상태 검증: 드래그한 아이템은 내 소유여야 하고, 드롭된 곳은 다른 사람이 잡고 있지 않아야 함
	if (SrcEntry.LockedByPlayerID != InPlayerID) return false;
	if (DstEntry.ItemRowName != NAME_None && DstEntry.LockedByPlayerID != INDEX_NONE) return false;

	// 목적지 슬롯이 이미 꽉 차 있다면 병합 불가능 (스왑으로 넘어가게 유도)
	if (DstEntry.CurCount >= MaxCount) return false;

	int32 TotalCount = SrcEntry.CurCount + DstEntry.CurCount;

	if (TotalCount <= MaxCount)
	{
		// 완벽 병합: 다 들어감
		DstEntry.CurCount = TotalCount;
		SrcEntry.Clear(); // 원본 슬롯은 빈칸 처리
	}
	else
	{
		// 일부 병합: 목적지는 꽉 채우고, 원본에 남은 개수 반환
		DstEntry.CurCount = MaxCount;
		SrcEntry.CurCount = TotalCount - MaxCount;
	}

	// 잠금 해제
	SrcEntry.LockedByPlayerID = INDEX_NONE;
	DstEntry.LockedByPlayerID = INDEX_NONE;

	// 변경 사항 알림 (FastArraySerializer 동기화)
	SrcComp->InventoryContainer.MarkItemDirty(SrcEntry);
	DstComp->InventoryContainer.MarkItemDirty(DstEntry);

	// 서버 UI 동기화 (리슨 서버용)
	SrcComp->OnInventorySlotChanged.Broadcast(SrcIdx, SrcEntry);
	DstComp->OnInventorySlotChanged.Broadcast(DstIdx, DstEntry);

	return true;
}


void UC_InvenComponent::StartDragItemSlot(int32 SlotIndex, int32 InPlayerId)
{
	if (!InventoryContainer.Items.IsValidIndex(SlotIndex)) return;
	
	if (InventoryContainer.Items[SlotIndex].ItemRowName == NAME_None) return;
	
	if (InventoryContainer.Items[SlotIndex].LockedByPlayerID != INDEX_NONE) return;
	
	
	InventoryContainer.Items[SlotIndex].LockedByPlayerID = InPlayerId;
	InventoryContainer.MarkItemDirty(InventoryContainer.Items[SlotIndex]);
	
	if (!GetOwner()->HasAuthority()) return;
	
	OnInventorySlotChanged.Broadcast(SlotIndex, InventoryContainer.Items[SlotIndex]);
}


void UC_InvenComponent::CancelDragItemSlot(int32 SlotIndex, int32 InPlayerId)
{
	UC_Util::Print("Cacel");
	if (!InventoryContainer.Items.IsValidIndex(SlotIndex))
		return;

	if (InventoryContainer.Items[SlotIndex].LockedByPlayerID != InPlayerId)
		return;
    
	InventoryContainer.Items[SlotIndex].LockedByPlayerID = INDEX_NONE;
	InventoryContainer.MarkItemDirty(InventoryContainer.Items[SlotIndex]);
	
	if (!GetOwner()->HasAuthority()) return;
	
	OnInventorySlotChanged.Broadcast(SlotIndex, InventoryContainer.Items[SlotIndex]);
}

void UC_InvenComponent::OnRep_InventoryContainer()
{
	InventoryContainer.OwnerComponent = this;
}
