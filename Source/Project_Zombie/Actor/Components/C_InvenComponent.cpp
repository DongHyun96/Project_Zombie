#include "Actor/Components/C_InvenComponent.h"

#include "C_EquippedComponent.h"
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

void UC_InvenComponent::LoadInventoryFromBackup(const TArray<FInventoryEntry>& InSavedItems)
{
	// 서버 권한 검사
	if (!GetOwner()->HasAuthority()) return;

	// Fast Array 내부의 TArray에 백업본 데이터 주입
	InventoryContainer.Items = InSavedItems;
    
	// 중요: Fast Array가 내부 요소를 모두 감지하여 클라이언트들에게 Replicate 하도록 마킹
	InventoryContainer.MarkArrayDirty();
    
	// 로컬 델리게이트 알림 혹은 강제 동기화 보정용 함수 호출
	//ForceRepInven();
    
	UE_LOG(LogTemp, Log, TEXT("[InvenComp] %d개의 아이템을 성공적으로 복구했습니다."), InventoryContainer.Items.Num());
}

void UC_InvenComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryContainer.OwnerComponent = this;

	if (GetOwner()->HasAuthority())
	{
		int32 TotalSlots = bHasEquipmentSlots ? (MaxSlots + static_cast<int32>(EWeaponSlot::None)) : MaxSlots;
		
		InitInventoryContainerMaxSlots(TotalSlots);
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
		
		int32 MaxCount = ItemManager->GetItemData<FItemData>(EItemTableType::General, SrcEntry.ItemRowName)->MaxCount;
           
		if (MaxCount > 1)
		{
			// 병합에 성공하면 조기 종료
			if (!TryMergeItem(SrcComp, SrcIdx, DstComp, DstIdx, InPlayerID, MaxCount)) return;
		}
	}

	// 병합을 할 수 없거나(다른 아이템, 꽉 참), 스택 불가 아이템인 경우 -> 무조건 스왑/이동
	SrcComp->SwapInvenEntry(SrcIdx, DstComp, DstIdx, InPlayerID);
}

int32 UC_InvenComponent::GetTotalItemCount(const FName& InItemRowName)
{
	int32 TotalItemCount = 0;
	FInventoryEntry Entry;
	for (int i = 0 ; i < InventoryContainer.Items.Num() ; ++i)
	{
		 Entry = InventoryContainer.Items[i];
		
		if (Entry.ItemRowName == InItemRowName)
		{
			TotalItemCount += Entry.CurCount;
		}
	}
	
	return TotalItemCount;
}

void UC_InvenComponent::SetEntryCurCount(int32 Idx, int32 InCount)
{
	InventoryContainer.Items[Idx].CurCount = InCount;
}


void UC_InvenComponent::InitInvenItemAt(int32 Idx)
{
	if (!GetOwner()->HasAuthority() || !InventoryContainer.Items.IsValidIndex(Idx)) return;

	InventoryContainer.Items[Idx].Clear();
	InventoryContainer.Items[Idx].SlotIndex = Idx; // 인덱스 유실 차단

	InventoryContainer.MarkItemDirty(InventoryContainer.Items[Idx]);
	OnInventorySlotChanged.Broadcast(Idx, InventoryContainer.Items[Idx]);
}

int32 UC_InvenComponent::AddItem(FInventoryEntry ItemEntry)
{
	if (ItemEntry.ItemRowName == NAME_None || ItemEntry.CurCount <= 0) return ItemEntry.CurCount;

	if (!GetWorld()) return ItemEntry.CurCount;
	
    UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
    
	if (!ItemManager) return ItemEntry.CurCount;

	const FItemData* PickUpItemData = ItemManager->GetItemData<FItemData>(EItemTableType::General, ItemEntry.ItemRowName); 
	
	if (!PickUpItemData) return ItemEntry.CurCount;
	
    int32 MaxCount = PickUpItemData->MaxCount;
    int32 RemainCount = ItemEntry.CurCount; // 넣어야 할 남은 수량

	// 장비 슬롯 유무에 따라 탐색 시작 인덱스 분기
	int32 StartIdx = bHasEquipmentSlots ? static_cast<int32>(EWeaponSlot::None) : 0;
	
    // 1. 기존에 존재하는 동일한 아이템 슬롯 찾아서 채워 넣기 (스택 가능 아이템)
    if (MaxCount > 1)
    {
        for (int32 i = StartIdx; i < MaxSlots; ++i)
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
    for (int32 i = StartIdx; i < MaxSlots; ++i)
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

bool UC_InvenComponent::CanSetItemToSlot(int32 TargetSlotIndex, const FInventoryEntry& Entry) const
{
	// 1. 이 인벤토리가 "장비 슬롯을 지원하는 인벤토리(플레이어)"이고
	//    목표 슬롯이 "장비 슬롯 구역(0~2번)"일 때만 타입 체크를 진행합니다.
	if (bHasEquipmentSlots && TargetSlotIndex < static_cast<int32>(EWeaponSlot::None)) //
	{
		// 빈 슬롯으로 만드는 것(장비 해제)은 항상 허용
		if (Entry.ItemRowName.IsNone()) return true;

		// Null Check (안전한 서브시스템 접근)
		UWorld* World = GetWorld();
		if (!World) return false;
		
		UGameInstance* GI = World->GetGameInstance();
		if (!GI) return false;

		UC_ItemManager* ItemManager = GI->GetSubsystem<UC_ItemManager>();
		if (!ItemManager) return false;
		
		// DataTable을 참조하여 해당 슬롯(주무기/보조무기/투척류)에 맞는 타입인지 검증
		const FItemData* ItemData = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);
		
		if (!ItemData) return false;
		
		switch (static_cast<EWeaponSlot>(TargetSlotIndex))
		{
		case EWeaponSlot::MainWeapon:
			return ItemData->ItemType == EItemType::MAINWEAPON;
		case EWeaponSlot::MeleeWeapon:
			return ItemData->ItemType == EItemType::MELEEWEAPON;
		case EWeaponSlot::ThrowableWeapon:
			return ItemData->ItemType == EItemType::THROWABLE;
		default:
			return false;
		}
	}

	// 2. 창고(bHasEquipmentSlots == false)이거나,
	//    플레이어의 일반 가방 구역(3번 이상)이라면 어떤 아이템이든 다 들어갈 수 있습니다!
	return true;
}

bool UC_InvenComponent::RemoveItemByRowName(FName InRowName, int32 InAmountCount)
{
	// 차감할 수량이 올바르지 않거나 빈 RowName이면 실패 처리
	if (InRowName.IsNone() || InAmountCount <= 0)
	{
		return false;
	}
	

	// 소유 중인 총 수량이 차감 요구량보다 적은지 미리 확인
	int32 TotalHeld = GetTotalItemCount(InRowName);
	if (TotalHeld < InAmountCount)
	{
		// 소유한 재료/아이템 부족
		return false; 
	}

	int32 RemainingToDeduct = InAmountCount;
    
	// InventoryContainer.Items 포인터 참조
	TArray<FInventoryEntry>* EntrySlots = &InventoryContainer.Items; 
	if (!EntrySlots) return false;

	// 인벤토리 슬롯 순회
	for (int32 i = 0; i < EntrySlots->Num(); ++i)
	{

		FInventoryEntry& Entry = (*EntrySlots)[i];

		// 비어있는 슬롯이거나 target RowName과 다른 아이템이면 스킵
		if (Entry.IsEmpty() || Entry.ItemRowName != InRowName)
		{
			continue;
		}

		// 현재 슬롯에서 차감할 개수 계산
		int32 DeductFromThisSlot = FMath::Min(Entry.CurCount, RemainingToDeduct);

		Entry.CurCount -= DeductFromThisSlot;
		RemainingToDeduct -= DeductFromThisSlot;

		
		// 슬롯의 수량이 0 이하가 되면 슬롯 초기화
		if (Entry.CurCount <= 0)
		{
			int32 CurSlotIdx = Entry.SlotIndex;
			
			Entry.Clear(); // 또는 Entry = FInventoryEntry();
			
			Entry.SlotIndex = CurSlotIdx;
		}
		// UI 업데이트 및 동기화를 위한 델리게이트 브로드캐스트
		
		if (GetOwner()->HasAuthority())
		{
			InventoryContainer.MarkItemDirty(Entry);
			OnInventorySlotChanged.Broadcast(i, Entry);
		}
		// 더 이상 차감할 수량이 없으면 성공 종료	
		if (RemainingToDeduct <= 0)
		{
			break;
		}
	}

	// 차감이 완전히 끝났는지 확인
	return RemainingToDeduct == 0;
}

void UC_InvenComponent::Server_RemoveItemByRowName_Implementation(FName InRowName, int32 InAmountCount)
{
	RemoveItemByRowName(InRowName, InAmountCount);
}


void UC_InvenComponent::ForceRepInven()
{
	InventoryContainer.MarkArrayDirty();
}

void UC_InvenComponent::ReleaseAllLocksByPlayer(int32 InPlayerID)
{
	if (!GetOwner()->HasAuthority()) return;
	if (InPlayerID == INDEX_NONE) return;

	const int32 NumItems = InventoryContainer.Items.Num();
	for (int32 i = 0; i < NumItems; ++i)
	{
		FInventoryEntry& Entry = InventoryContainer.Items[i];

		if (Entry.LockedByPlayerID == InPlayerID)
		{
			// 무거운 검증 단계 우회하고 직접 안전 세터로 해제 처리
			SetSlotLockState(i, INDEX_NONE);
            
			UE_LOG(LogTemp, Log, TEXT("Inven [%s] Slot [%d] Lock Released Successfully for Player [%d]!"), 
			   *GetName(), i, InPlayerID);
		}
	}
}

void UC_InvenComponent::ForceReleaseSlotLock(int32 SlotIndex)
{
	SetSlotLockState(SlotIndex, INDEX_NONE);
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
	if (!InventoryContainer.Items.IsValidIndex(MySlotIdx) || !TargetComp->InventoryContainer.Items.IsValidIndex(TargetSlotIdx)) return false;

	FInventoryEntry& MyEntry = InventoryContainer.Items[MySlotIdx];
	FInventoryEntry& TargetEntry = TargetComp->InventoryContainer.Items[TargetSlotIdx];

	// 2. 잠금 검증
	if (MyEntry.LockedByPlayerID != InPlayerID) return false;
	if (TargetEntry.ItemRowName != NAME_None && TargetEntry.LockedByPlayerID != INDEX_NONE) return false;

	// 3. 구조체 전체 대입을 통한 안전한 데이터 스왑 (확장성 확보)
	FInventoryEntry TempEntry = MyEntry;
	MyEntry = TargetEntry;
	TargetEntry = TempEntry;

	// 4. 고유 식별 정보인 슬롯 인덱스는 원래대로 복구
	MyEntry.SlotIndex = MySlotIdx;
	TargetEntry.SlotIndex = TargetSlotIdx;
	
	// 5. 잠금 해제 (이 내부에서 MarkItemDirty와 Broadcast가 수행됨)
	SetSlotLockState(MySlotIdx, INDEX_NONE);
	TargetComp->SetSlotLockState(TargetSlotIdx, INDEX_NONE);

	return true;
}

bool UC_InvenComponent::TryMergeItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx, int32 InPlayerID, int32 MaxCount)
{
	if (!GetOwner()->HasAuthority() || !SrcComp || !DstComp) return false;

	FInventoryEntry& SrcEntry = SrcComp->InventoryContainer.Items[SrcIdx];
	FInventoryEntry& DstEntry = DstComp->InventoryContainer.Items[DstIdx];

	// 잠금 상태 검증
	if (SrcEntry.LockedByPlayerID != InPlayerID)
	{
		SrcComp->SetSlotLockState(SrcIdx, INDEX_NONE);
		DstComp->SetSlotLockState(DstIdx, INDEX_NONE);
		return false;
	}
	if (DstEntry.ItemRowName != NAME_None && DstEntry.LockedByPlayerID != INDEX_NONE)
	{
		SrcComp->SetSlotLockState(SrcIdx, INDEX_NONE);
		DstComp->SetSlotLockState(DstIdx, INDEX_NONE);
		return false;
	}
	if (DstEntry.CurCount >= MaxCount)
	{
		SrcComp->SetSlotLockState(SrcIdx, INDEX_NONE);
		DstComp->SetSlotLockState(DstIdx, INDEX_NONE);
		return false;
	}

	int32 TotalCount = SrcEntry.CurCount + DstEntry.CurCount;

	if (TotalCount <= MaxCount)
	{
		// 완벽 병합: 다 들어감
		DstEntry.CurCount = TotalCount;
		SrcEntry.Clear(); // 원본 슬롯은 빈칸 처리 (이 안에서 개수와 필드가 초기화되지만 SlotIndex는 유지되어야 함)
		SrcEntry.SlotIndex = SrcIdx; // Clear 후 혹시 모를 인덱스 유실 방지
	}
	else
	{
		// 일부 병합: 목적지는 꽉 채우고, 원본에 남은 개수 반환
		DstEntry.CurCount = MaxCount;
		SrcEntry.CurCount = TotalCount - MaxCount;
	}

	// 데이터 변경이 일어났으므로 명시적으로 Dirty 마크 및 브로드캐스트 수행
	SrcComp->InventoryContainer.MarkItemDirty(SrcEntry);
	DstComp->InventoryContainer.MarkItemDirty(DstEntry);
    
	SrcComp->OnInventorySlotChanged.Broadcast(SrcIdx, SrcEntry);
	DstComp->OnInventorySlotChanged.Broadcast(DstIdx, DstEntry);

	// 잠금 해제 
	SrcComp->SetSlotLockState(SrcIdx, INDEX_NONE);
	DstComp->SetSlotLockState(DstIdx, INDEX_NONE);
	return true;
}

void UC_InvenComponent::InitInventoryContainerMaxSlots(int32 InMax)
{
	// 서버는 무조건 45개로 초기화 (기존 데이터가 있다면 덮어씀)
	InventoryContainer.Items.SetNum(InMax);
       
	for (int32 i = 0; i < InMax; ++i)
	{
		// 데이터가 비어있을 때만 Clear, 이미 데이터가 있다면(세이브 로드 등) 건드리지 않음
		if (InventoryContainer.Items[i].ItemRowName == NAME_None)
		{
			InventoryContainer.Items[i].Clear();
		}
		InventoryContainer.Items[i].SlotIndex = i;
	}
		
	InventoryContainer.MarkArrayDirty();
}

bool UC_InvenComponent::ProcessItemDivideMove(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp,
                                              int32 DstIdx, int32 SplitCount, int32 InPlayerID)
{
	if (!GetOwner()->HasAuthority() || !SrcComp || !DstComp) return false;

    FInventoryEntry& SrcEntry = SrcComp->InventoryContainer.Items[SrcIdx];
    FInventoryEntry& DstEntry = DstComp->InventoryContainer.Items[DstIdx];

    // 1. 잠금 검증 및 기본 유효성 검사
    if (SrcEntry.LockedByPlayerID != InPlayerID) return false;
    //if (DstEntry.ItemRowName != NAME_None && DstEntry.LockedByPlayerID != INDEX_NONE) return false;
    if (SrcEntry.ItemRowName == NAME_None || SrcEntry.CurCount < SplitCount) return false;

    // 2. 목적지 슬롯 상태에 따른 분기
    if (DstEntry.ItemRowName == NAME_None)
    {
        // [A] 목적지가 빈 슬롯인 경우 -> 그대로 다 채워 넣기
    	SrcEntry.CurCount -= SplitCount;
    	
    	DstEntry = SrcEntry; 
    	DstEntry.CurCount = SplitCount;
    	DstEntry.SlotIndex = DstIdx; // 슬롯 인덱스는 본인 것으로 복구
    	DstEntry.LockedByPlayerID = INDEX_NONE; // 드롭 위치는 락 해제 상태로
    
    }
    else if (DstEntry.ItemRowName == SrcEntry.ItemRowName)
    {
        // [B] 목적지에 동일한 아이템이 존재하고 병합이 가능한 경우
        UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
        if (!ItemManager) return false;

        int32 MaxCount = ItemManager->GetItemData<FItemData>(EItemTableType::General, SrcEntry.ItemRowName)->MaxCount;
        
        // 목적지가 이미 풀스택이면 처리 불가 (애초에 진입 금지)
        if (DstEntry.CurCount >= MaxCount) return false;

        // 넣을 수 있는 만큼만(여유 공간만큼) 계산
        int32 AcceptableCount = FMath::Min(SplitCount, MaxCount - DstEntry.CurCount);

        DstEntry.CurCount += AcceptableCount;
        SrcEntry.CurCount -= AcceptableCount;
    }
    else
    {
        // [C] 다른 아이템이 있는 경우 -> 분할 불가 (스왑 불가능하므로 무효 처리)
        return false;
    }

    // 3. 원본 슬롯이 완전히 비었으면 청소
    if (SrcEntry.CurCount <= 0)
    {
        SrcEntry.Clear();
    	SrcEntry.SlotIndex = SrcIdx;
    }

    // 4. 잠금 해제 및 변경 동기화
	SrcComp->InventoryContainer.MarkItemDirty(SrcEntry);
	DstComp->InventoryContainer.MarkItemDirty(DstEntry);
	SrcComp->OnInventorySlotChanged.Broadcast(SrcIdx, SrcEntry);
	DstComp->OnInventorySlotChanged.Broadcast(DstIdx, DstEntry);

	// 잠금 해제는 전용 Setter를 통해 안전하게 마무리합니다 (여기서 중복 패킷 방지 조건 처리됨).
	SrcComp->SetSlotLockState(SrcIdx, INDEX_NONE);
	DstComp->SetSlotLockState(DstIdx, INDEX_NONE);

    return true;
}

int32 UC_InvenComponent::ProcessItemDivideDrop(int32 SrcIdx, int32 SplitCount, int32 InPlayerID)
{
	if (!GetOwner()->HasAuthority() || SplitCount <= 0) return 0;
	if (!InventoryContainer.Items.IsValidIndex(SrcIdx)) return 0;

	FInventoryEntry& SrcEntry = InventoryContainer.Items[SrcIdx];

	// 1. 유효성 및 잠금 상태 검증
	if (SrcEntry.ItemRowName == NAME_None) return 0;
	if (SrcEntry.LockedByPlayerID != InPlayerID) return 0;
	if (SrcEntry.CurCount < SplitCount) return 0; // 요청한 개수가 가지고 있는 것보다 많으면 실패

	// 2. 인벤토리 데이터 차감 처리
	SrcEntry.CurCount -= SplitCount;

	if (SrcEntry.CurCount <= 0)
	{
		SrcEntry.Clear(); // Clear 내부에서 LockedByPlayerID는 INDEX_NONE이 되지만 아래 SetSlotLockState 채널 통일을 위해 둠
		SrcEntry.SlotIndex = SrcIdx; // 인덱스 보존
	}

	// 데이터 변화에 따른 동기화 우선 수행
	InventoryContainer.MarkItemDirty(SrcEntry);
	OnInventorySlotChanged.Broadcast(SrcIdx, SrcEntry);

	// 잠금 해제는 이 하나의 경로로 완전히 위임
	SetSlotLockState(SrcIdx, INDEX_NONE);

	// 검증과 차감이 완벽히 끝났으므로, 드롭을 진행할 개수 반환
	return SplitCount;
}

bool UC_InvenComponent::SetSlotLockState(int32 SlotIdx, int32 InPlayerID)
{
	// 1. 유효성 검사 (서버 권한 및 인덱스 범위)
	if (!GetOwner()->HasAuthority()) return false;
	if (!InventoryContainer.Items.IsValidIndex(SlotIdx)) return false;

	FInventoryEntry& Entry = InventoryContainer.Items[SlotIdx];

	// 값의 변화가 없다면 불필요한 네트워크 패킷 전송(Dirty 마크) 방지
	//if (Entry.LockedByPlayerID == InPlayerID) return true;

	// 2. 잠금 값 대입
	Entry.LockedByPlayerID = InPlayerID;

	// 3. FastArraySerializer 동기화 마크 및 로컬 UI 브로드캐스트
	InventoryContainer.MarkItemDirty(Entry);
	// TODO : 여기서 클라이언트에게 cursorItem의 초기화가 필요해 보임.
	
	OnInventorySlotChanged.Broadcast(SlotIdx, Entry);

	return true;
}


void UC_InvenComponent::StartDragItemSlot(int32 SlotIndex, int32 InPlayerId)
{
	if (!GetOwner()->HasAuthority() || !InventoryContainer.Items.IsValidIndex(SlotIndex)) return;
	
	if (InventoryContainer.Items[SlotIndex].ItemRowName == NAME_None) return;
	
	if (InventoryContainer.Items[SlotIndex].LockedByPlayerID != INDEX_NONE) return;
	
	SetSlotLockState(SlotIndex, InPlayerId);
}


void UC_InvenComponent::CancelDragItemSlot(int32 SlotIndex, int32 InPlayerId)
{
	if (!GetOwner()->HasAuthority() || !InventoryContainer.Items.IsValidIndex(SlotIndex)) return;

	if (InventoryContainer.Items[SlotIndex].LockedByPlayerID != InPlayerId) return;
    
	SetSlotLockState(SlotIndex, INDEX_NONE);
}

void UC_InvenComponent::OnRep_InventoryContainer()
{
	InventoryContainer.OwnerComponent = this;
	//InventoryContainer.MarkArrayDirty();
}
