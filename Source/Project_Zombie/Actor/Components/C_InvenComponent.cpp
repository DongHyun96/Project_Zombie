#include "Actor/Components/C_InvenComponent.h"
#include "Utility/C_Util.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "Net/UnrealNetwork.h"
UC_InvenComponent::UC_InvenComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

	InventoryItems.SetNum(MaxSlots);
	// 컴포넌트 리플리케이션 활성화. 
	SetIsReplicatedByDefault(true);
}


bool UC_InvenComponent::SwapInvenEntry(int32 SlotIdx1, int32 SlotIdx2)
{
	UC_Util::Print(SlotIdx1);				 // 드롭된 슬롯
	UC_Util::Print(SlotIdx2, FColor::Green); // 드래그된 슬롯
	
	if (!InventoryItems.IsValidIndex(SlotIdx1) || !InventoryItems.IsValidIndex(SlotIdx2))
	{
		return false;
	}
    
	// 같은 슬롯이면 바꿀 필요가 없으므로 무조건 true 반환
	if (SlotIdx1 == SlotIdx2) return false;
	

	// 데이터 교환 (언리얼 내장 함수 사용으로 한 줄로 단축)
	InventoryItems.Swap(SlotIdx1, SlotIdx2);
	
	OnInventorySlotChanged.Broadcast(SlotIdx1, InventoryItems[SlotIdx1]);
	OnInventorySlotChanged.Broadcast(SlotIdx2, InventoryItems[SlotIdx2]);
	
	return true;
}

void UC_InvenComponent::InitInvenItemAt(int32 idx)
{
	InventoryItems[idx].Initialize();
}

void UC_InvenComponent::BeginPlay()
{
	Super::BeginPlay();

	// ◀ 컴포넌트 자체가 네트워크 복제가 되도록 설정해야 내부 변수도 복제됩니다.
	//SetIsReplicatedByDefault(true);
}


//void UC_InvenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//}

bool UC_InvenComponent::AddItem(FInventoryEntry ItemEntry)
{
	// TODO : 멀티 환경 적용하기, 더 좋은 방법이 분명 존재해 보임.
	// 쌓을 수 있는 아이템이라면
	//if (ItemEntry.bIsStack == false)
	//{
	//	for (int i = 0; i < MaxSlots; ++i)
	//	{
	//		// 빈칸이면 바로 아이템 넣기
	//		if (InventoryItems[i].ItemRowName == NAME_None)
	//		{
	//			InventoryItems[i] = ItemEntry;
	//			break;
	//		}
	//	}

	//	return false;
	//}
	//// 쌓을 수 없는 아이템이라면
	//else
	//{
	//	int FirstEmptySlotNum = MaxSlots;
	//	for (int i = 0; i < MaxSlots; ++i)
	//	{
	//		// 같은 아이템을 찾았다면 Count 더해주기
	//		if (InventoryItems[i].ItemRowName == ItemEntry.ItemRowName)
	//		{
	//			InventoryItems[i].Count += ItemEntry.Count;
	//			break;
	//		}
	//		// 같은 아이템이 없는 경우를 고려해서 
	//		else if (InventoryItems[i].ItemRowName == NAME_None)
	//		{
	//			FirstEmptySlotNum = FMath::Min(FirstEmptySlotNum, i);
	//		}
	//	}
	//	
	//	if (FirstEmptySlotNum == MaxSlots)
	//		return false; // MaxSlots이면 인벤에 못넣는 상태.(idx : 0~MaxSlots-1 ; num : MaxSlots)
	//	else 
	//		InventoryItems[FirstEmptySlotNum] = ItemEntry;
	//}

	//return true;

	int32 TargetIndex = -1;

	// 1. 쌓을 수 있는 아이템인 경우 -> 기존에 같은 아이템이 있는지 먼저 검색
	if (ItemEntry.bIsStack)
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (InventoryItems[i].ItemRowName == ItemEntry.ItemRowName)
			{
				InventoryItems[i].Count += ItemEntry.Count;
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
			if (InventoryItems[i].ItemRowName == NAME_None)
			{
				InventoryItems[i] = ItemEntry;
				TargetIndex = i;
				break; // 빈 칸에 넣었으니 즉시 종료
			}
		}
	}

	// 3. 결과 처리
	if (TargetIndex != -1)
	{
		// UI 및 리스너들에게 변경 사항 브로드캐스트
		
		OnInventorySlotChanged.Broadcast(TargetIndex, InventoryItems[TargetIndex]);
		UC_Util::Print(InventoryItems[TargetIndex].ItemRowName.ToString());

		UC_Util::Print(InventoryItems[TargetIndex].Count);
		return true;
	}

	// 인벤토리가 가득 차서 공간이 없음
	return false;
}

void UC_InvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [최적화] 방장의 부담을 줄이기 위해, 해당 인벤토리의 소유자(클라이언트)에게만 복제
	DOREPLIFETIME_CONDITION(UC_InvenComponent, InventoryItems, COND_OwnerOnly);
}

// 서버에서 데이터가 바뀌어 클라이언트로 넘어왔을 때 UI 갱신 알림
void UC_InvenComponent::OnRep_InventoryItems()
{
	for (int32 i = 0; i < InventoryItems.Num(); ++i)
	{
		OnInventorySlotChanged.Broadcast(i, InventoryItems[i]);
	}
}

// 서로 다른 인벤토리 컴포넌트 간의 아이템 스왑/이동
bool UC_InvenComponent::TransferItemTo(int32 MySlotIdx, UC_InvenComponent* TargetComp, int32 TargetSlotIdx)
{
	if (!InventoryItems.IsValidIndex(MySlotIdx) || !TargetComp || !TargetComp->InventoryItems.IsValidIndex(TargetSlotIdx)) return false;

	FInventoryEntry TempItem = InventoryItems[MySlotIdx];
	InventoryItems[MySlotIdx] = TargetComp->InventoryItems[TargetSlotIdx];
	TargetComp->InventoryItems[TargetSlotIdx] = TempItem;

	// 양쪽 인벤토리 모두에게 변경 사항 브로드캐스트
	this->OnInventorySlotChanged.Broadcast(MySlotIdx, InventoryItems[MySlotIdx]);
	TargetComp->OnInventorySlotChanged.Broadcast(TargetSlotIdx, TargetComp->InventoryItems[TargetSlotIdx]);
    
	return true;
}

// 클라이언트 -> 서버 이동 요청 RPC 구현부
bool UC_InvenComponent::Server_RequestMoveItem_Validate(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx)
{
	if (!SrcComp || !DstComp) return false;
	return true;
}

void UC_InvenComponent::Server_RequestMoveItem_Implementation(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx)
{
	if (SrcComp == DstComp)
	{
		SrcComp->SwapInvenEntry(SrcIdx, DstIdx);
	}
	else
	{
		SrcComp->TransferItemTo(SrcIdx, DstComp, DstIdx);
	}
}