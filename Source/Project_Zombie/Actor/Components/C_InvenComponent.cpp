#include "Actor/Components/C_InvenComponent.h"
#include "Utility/C_Util.h"
#include "GameMode/C_UIManager.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/InvenUI/C_ItemSlotWidget.h"
UC_InvenComponent::UC_InvenComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

	InventoryItems.SetNum(MaxSlots);
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
	if (SlotIdx1 == SlotIdx2) return true;

	// 1. 소유자 액터를 APawn으로 형변환 후 플레이어 컨트롤러 가져오기
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return false;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return false;

	// 2. 컨트롤러가 가진 HUD를 AC_UIManager로 형변환
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	if (!UIManager) return false;

	// 3. 인벤토리 위젯 가져오기
	UC_InventoryWidget* InvWidget = UIManager->GetInventoryWidget();
	if (!InvWidget) return false;

	// 4. 데이터 교환 (언리얼 내장 함수 사용으로 한 줄로 단축)
	InventoryItems.Swap(SlotIdx1, SlotIdx2);

	// 5. UI 새로고침 (GridWidget과 SlotArr, 개별 슬롯의 유효성 검사도 곁들이면 안전합니다)
	if (InvWidget->GetGridWidget() && !InvWidget->GetGridWidget()->GetSlotArr().IsEmpty())
	{
		UC_Util::Print(SlotIdx1);
		UC_Util::Print(SlotIdx2);
		InvWidget->GetGridWidget()->RefreshSlotAt(SlotIdx1, InventoryItems[SlotIdx1]); 
		InvWidget->GetGridWidget()->RefreshSlotAt(SlotIdx2, InventoryItems[SlotIdx2]); 
	}

	// [추가] 성공적으로 끝났으므로 true 반환
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

//void UC_InvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	// InventoryItems 배열이 서버에서 클라이언트로 복제되도록 등록
//	DOREPLIFETIME(UC_InvenComponent, InventoryItems);
//}

