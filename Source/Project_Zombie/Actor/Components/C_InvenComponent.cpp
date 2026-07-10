#include "Actor/Components/C_InvenComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
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

bool UC_InvenComponent::SwapInvenEntry(int32 SlotIdx1, int32 SlotIdx2, int32 InPlayerId)
{
	if (!GetOwner()->HasAuthority()) return false; // 서버가 아니면 컷
	if (!InventoryContainer.Items.IsValidIndex(SlotIdx1) || !InventoryContainer.Items.IsValidIndex(SlotIdx2)) return false;
	if (SlotIdx1 == SlotIdx2) return false;
	
	// 원본 슬롯은 반드시 내가 잠근 슬롯이어야 함
	if (InventoryContainer.Items[SlotIdx1].LockedByPlayerID != InPlayerId) return false;

	// 목적지 슬롯은 다른 사람이 잠그지 않았어야 함
	if (InventoryContainer.Items[SlotIdx2].LockedByPlayerID != INDEX_NONE) return false;

	// 데이터 교환 (SlotIndex 주소값은 유지하고 알맹이 데이터만 스왑), IsDragging(혹은 bIsLocked)는 둘 이 시점에 둘 다 false여야 해서 굳이 복사 안함.
	FName TempRowName = InventoryContainer.Items[SlotIdx1].ItemRowName;
	int32 TempCount = InventoryContainer.Items[SlotIdx1].Count;
	int32 TempUpgrade = InventoryContainer.Items[SlotIdx1].UpgradeLevel;
	int32 TempAmmo = InventoryContainer.Items[SlotIdx1].CurAmmo;

	InventoryContainer.Items[SlotIdx1].ItemRowName = InventoryContainer.Items[SlotIdx2].ItemRowName;
	InventoryContainer.Items[SlotIdx1].Count = InventoryContainer.Items[SlotIdx2].Count;
	InventoryContainer.Items[SlotIdx1].UpgradeLevel = InventoryContainer.Items[SlotIdx2].UpgradeLevel;
	InventoryContainer.Items[SlotIdx1].CurAmmo = InventoryContainer.Items[SlotIdx2].CurAmmo;

	InventoryContainer.Items[SlotIdx2].ItemRowName = TempRowName;
	InventoryContainer.Items[SlotIdx2].Count = TempCount;
	InventoryContainer.Items[SlotIdx2].UpgradeLevel = TempUpgrade;
	InventoryContainer.Items[SlotIdx2].CurAmmo = TempAmmo;

	// 잠금 해제
	SetItemLock(SlotIdx1);
	SetItemLock(SlotIdx2);
	
	// 언리얼 엔진에게 바뀐 슬롯 2개만 선별해서 패킷 쏘라고 지시 (클라이언트 자동 연동)
	InventoryContainer.MarkItemDirty(InventoryContainer.Items[SlotIdx1]);
	InventoryContainer.MarkItemDirty(InventoryContainer.Items[SlotIdx2]);

	if (!GetOwner()->HasAuthority()) return true;
	
	// 서버(리슨서버 방장) 로컬 UI도 즉시 동기화
	OnInventorySlotChanged.Broadcast(SlotIdx1, InventoryContainer.Items[SlotIdx1]);
	OnInventorySlotChanged.Broadcast(SlotIdx2, InventoryContainer.Items[SlotIdx2]);

	return true;
}

// 서로 다른 인벤토리 컴포넌트 간의 아이템 스왑/이동
bool UC_InvenComponent::TransferItemTo(int32 MySlotIdx, UC_InvenComponent* TargetComp, int32 TargetSlotIdx, int32 InPlayerId)
{
	if (!GetOwner()->HasAuthority() || !TargetComp) return false;
	if (!InventoryContainer.Items.IsValidIndex(MySlotIdx) || !TargetComp->InventoryContainer.Items.IsValidIndex(TargetSlotIdx)) return false;

	// 원본 슬롯이 드래그한 플레이어의 ID가 아니라면 차단.
	if (InventoryContainer.Items[MySlotIdx].LockedByPlayerID != InPlayerId) return false;
	
	// 드롭된 아이템 슬롯의 아이템이 잠김 상태라면 차단.
	if (TargetComp->GetInventoryItems()[TargetSlotIdx].LockedByPlayerID != INDEX_NONE) return false;
	
	// 내 가방 데이터 백업, IsDragging(혹은 bIsLocked)는 둘 이 시점에 둘 다 false여야 해서 굳이 복사 안함.
	FName TempRowName = InventoryContainer.Items[MySlotIdx].ItemRowName;
	int32 TempCount = InventoryContainer.Items[MySlotIdx].Count;
	int32 TempUpgrade = InventoryContainer.Items[MySlotIdx].UpgradeLevel;
	int32 TempAmmo = InventoryContainer.Items[MySlotIdx].CurAmmo;
	
	// 내 가방 <- 타겟(창고) 데이터 복사
	InventoryContainer.Items[MySlotIdx].ItemRowName = TargetComp->InventoryContainer.Items[TargetSlotIdx].ItemRowName;
	InventoryContainer.Items[MySlotIdx].Count = TargetComp->InventoryContainer.Items[TargetSlotIdx].Count;
	InventoryContainer.Items[MySlotIdx].UpgradeLevel = TargetComp->InventoryContainer.Items[TargetSlotIdx].UpgradeLevel;
	InventoryContainer.Items[MySlotIdx].CurAmmo = TargetComp->InventoryContainer.Items[TargetSlotIdx].CurAmmo;

	// 타겟(창고) <- 내 가방 백업 데이터 복사
	TargetComp->InventoryContainer.Items[TargetSlotIdx].ItemRowName = TempRowName;
	TargetComp->InventoryContainer.Items[TargetSlotIdx].Count = TempCount;
	TargetComp->InventoryContainer.Items[TargetSlotIdx].UpgradeLevel = TempUpgrade;
	TargetComp->InventoryContainer.Items[TargetSlotIdx].CurAmmo = TempAmmo;

	// 잠금 해제
	SetItemLock(MySlotIdx);
	TargetComp->SetItemLock(TargetSlotIdx);
	
	// 양쪽 인벤토리의 변경된 슬롯 마킹 (각각 알아서 최적화되어 날아감)
	InventoryContainer.MarkItemDirty(InventoryContainer.Items[MySlotIdx]);
	TargetComp->InventoryContainer.MarkItemDirty(TargetComp->InventoryContainer.Items[TargetSlotIdx]);
	
	if (GetOwner()->HasAuthority())
	{
		// 서버(리슨서버 방장) 로컬 UI 즉시 동기화
		OnInventorySlotChanged.Broadcast(MySlotIdx, InventoryContainer.Items[MySlotIdx]);
		TargetComp->OnInventorySlotChanged.Broadcast(TargetSlotIdx, TargetComp->InventoryContainer.Items[TargetSlotIdx]);
	}
	
	return true;
}

void UC_InvenComponent::InitInvenItemAt(int32 idx)
{
	InventoryContainer.Items[idx].Clear();
}

bool UC_InvenComponent::AddItem(FInventoryEntry ItemEntry)
{
	// TODO : 멀티 환경 적용하기, 더 좋은 방법이 분명 존재해 보임.

	int32 TargetIndex = -1;

	// 1. 쌓을 수 있는 아이템인 경우 -> 기존에 같은 아이템이 있는지 먼저 검색
	if (ItemEntry.bIsStack)
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (InventoryContainer.Items[i].ItemRowName == ItemEntry.ItemRowName)
			{
				InventoryContainer.Items[i].Count += ItemEntry.Count;
				TargetIndex = i;
				break; // 찾았으니 즉시 종료
			}
		}
	}

	// 2. 새로운 아이템이거나 쌓을 수 없는 아이템인 경우 -> 빈 슬롯 찾기
	if (TargetIndex == -1)
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (InventoryContainer.Items[i].ItemRowName == NAME_None)
			{
				InventoryContainer.Items[i] = ItemEntry;
				InventoryContainer.Items[i].SlotIndex = i;
				TargetIndex = i;
				break; // 빈 칸에 넣었으니 즉시 종료
			}
		}
	}

	// 3. 결과 처리
	if (TargetIndex != -1)
	{
		// UI 및 리스너들에게 변경 사항 브로드캐스트
		if (GetOwner()->HasAuthority())
		{
			OnInventorySlotChanged.Broadcast(TargetIndex, InventoryContainer.Items[TargetIndex]);
		}
		InventoryContainer.MarkItemDirty(InventoryContainer.Items[TargetIndex]);
		UC_Util::Print(InventoryContainer.Items[TargetIndex].ItemRowName.ToString());

		UC_Util::Print(InventoryContainer.Items[TargetIndex].Count);
		return true;
	}

	// 인벤토리가 가득 차서 공간이 없음
	return false;
}

void UC_InvenComponent::ForceRepInven()
{
	InventoryContainer.MarkArrayDirty();
}



void UC_InvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [최적화] 방장의 부담을 줄이기 위해, 해당 인벤토리의 소유자(클라이언트)에게만 복제
	//DOREPLIFETIME_CONDITION(UC_InvenComponent, InventoryContainer, COND_OwnerOnly);
	
	DOREPLIFETIME(UC_InvenComponent, InventoryContainer);
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
