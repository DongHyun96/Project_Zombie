#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "Components/UniformGridPanel.h"
#include "C_ItemSlotWidget.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Utility/C_Util.h"

void UC_InventoryGridWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UC_InventoryGridWidget::RefreshAllSlots(const TArray<FInventoryEntry>& InventoryItems)
{
    UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;

    // 데이터 배열과 위젯 배열의 크기 중 작은 것을 기준으로 안전하게 순회
    int32 LoopCount = FMath::Min(SlotWidgets.Num(), InventoryItems.Num());


    for (int32 i = 0; i < LoopCount; ++i)
    {
        if (!SlotWidgets[i]) continue;
        
        SlotWidgets[i]->SetAssociatedComponent(InvenComp);
        const FInventoryEntry& Entry = InventoryItems[i];

        // 아이템 매니저를 통해 데이터 테이블의 원본 비주얼/기본 스펙 데이터를 가져옴
        const FItemData* CoreData = ItemManager->GetItemData(Entry.ItemRowName);
        //if (!CoreData) continue; // TODO : 인벤에 없는 데이터인 경우 따로 처리해주기.
        // 실시간 인스턴스 데이터(Entry)와 원본 스펙 데이터(CoreData)를 함께 넘겨줌
        SlotWidgets[i]->UpdateSlot(Entry, CoreData);
    }
}

void UC_InventoryGridWidget::RefreshSlotAt(int32 SlotIndex, const FInventoryEntry& ItemData)
{
    UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;

    if (SlotWidgets.IsValidIndex(SlotIndex) && SlotWidgets[SlotIndex])
    {
        const FItemData* CoreData = ItemManager->GetItemData(ItemData.ItemRowName);

        SlotWidgets[SlotIndex]->UpdateSlot(ItemData, CoreData);
    }
}

void UC_InventoryGridWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // 에디터에서 컴포넌트나 클래스가 제대로 세팅되었는지 최소한의 방어선만 확인
    if (!ItemGridPanel || !SlotWidgetClass) return;

    // [조건문 없음] 평생 딱 한 번만 실행되므로 그냥 냅다 만듭니다.
    ItemGridPanel->ClearChildren();
    SlotWidgets.Reset();

    for (int32 i = 0; i < MaxSlots; ++i)
    {
        UC_ItemSlotWidget* NewSlot = CreateWidget<UC_ItemSlotWidget>(this, SlotWidgetClass);
        //if (!NewSlot) continue;

        NewSlot->SetSlotIndex(i); 
        NewSlot->SetGridWidget(this);

        int32 CurRow = i / Column;
        int32 CurColumn = i % Column;
        
        ItemGridPanel->AddChildToUniformGrid(NewSlot, CurRow, CurColumn);
        SlotWidgets.Add(NewSlot);
    }

    return;
}

/*bool UC_InventoryGridWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    // 에디터에서 컴포넌트나 클래스가 제대로 세팅되었는지 최소한의 방어선만 확인
    if (!ItemGridPanel || !SlotWidgetClass) return true;

    // [조건문 없음] 평생 딱 한 번만 실행되므로 그냥 냅다 만듭니다.
    ItemGridPanel->ClearChildren();
    SlotWidgets.Reset();

    for (int32 i = 0; i < MaxSlots; ++i)
    {
        UC_ItemSlotWidget* NewSlot = CreateWidget<UC_ItemSlotWidget>(this, SlotWidgetClass);
        //if (!NewSlot) continue;

        NewSlot->SetSlotIndex(i); 
        NewSlot->SetGridWidget(this);

        int32 CurRow = i / Column;
        int32 CurColumn = i % Column;
        
        ItemGridPanel->AddChildToUniformGrid(NewSlot, CurRow, CurColumn);
        SlotWidgets.Add(NewSlot);
    }

    return true;
}*/

void UC_InventoryGridWidget::SetInvenComponent(class UC_InvenComponent* InventoryComponent)
{
    if (InvenComp)
    {
        InvenComp->OnInventorySlotChanged.RemoveDynamic(this, &UC_InventoryGridWidget::RefreshSlotAt);
    }
    
    // 컴포넌트 교체
    InvenComp = InventoryComponent;
    
    // [새로운 바인딩 등록] 새로 들어온 컴포넌트가 유효하다면 연결을 맺고 갱신합니다.
    if (InvenComp)
    {
        // 중복 등록 방지를 위해 한 번 더 확실히 지우고 등록하는 안전장치
        InvenComp->OnInventorySlotChanged.RemoveDynamic(this, &UC_InventoryGridWidget::RefreshSlotAt);
        InvenComp->OnInventorySlotChanged.AddDynamic(this, &UC_InventoryGridWidget::RefreshSlotAt);
        
        // 전체 슬롯 다시 그리기
        RefreshAllSlots(InvenComp->GetInventoryItems());
    }
    else
    {
        // 만약 nullptr이 들어왔다면(창고에서 멀어졌다면) UI의 흔적을 청소해 줍니다.
        // (필요에 따라 빈 배열을 넘겨 슬롯을 다 비우거나 비활성화 처리)
        TArray<FInventoryEntry> EmptyArray;
        RefreshAllSlots(EmptyArray);
    }
}

