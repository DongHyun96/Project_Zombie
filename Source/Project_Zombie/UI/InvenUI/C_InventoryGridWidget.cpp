#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "Components/UniformGridPanel.h"
#include "C_GridItemSlotWidget.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_EquippedComponent.h"
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

    // UI 슬롯 위젯 기준으로 순회하여 실제 InvenComponent의 아이템 데이터를 매핑
    for (UC_GridItemSlotWidget* SlotWidget : SlotWidgets)
    {
        if (!SlotWidget) continue;

        SlotWidget->SetAssociatedComponent(InvenComp);

        int32 TargetSlotIndex = SlotWidget->GetSlotIndex();

        // InvenComponent의 배열 범위 내에 있는지 안전하게 체크
        if (InvenComp && InventoryItems.IsValidIndex(TargetSlotIndex))
        {
            const FInventoryEntry& Entry = InventoryItems[TargetSlotIndex];
            const FItemData* CoreData = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);

            SlotWidget->UpdateSlot(Entry, CoreData);
        }
        //else
        //{
        //    // 범위를 벗어나거나 아이템이 없으면 빈 슬롯으로 초기화
        //    SlotWidget->UpdateSlot(FInventoryEntry(), nullptr);
        //}
    }
}

void UC_InventoryGridWidget::RefreshSlotAt(int32 SlotIndex, const FInventoryEntry& ItemData)
{
    UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;

    int32 TargetSlotIndex = SlotIndex - SlotStartIdx;
    
    // 내 범위(3 ~ 47)가 아니면 0.0001초만에 즉시 리턴
    if (SlotIndex < SlotStartIdx || SlotIndex >= SlotStartIdx + MaxSlots) return;
    
    if (SlotWidgets.IsValidIndex(TargetSlotIndex) && SlotWidgets[TargetSlotIndex])
    {
        const FItemData* CoreData = ItemManager->GetItemData<FItemData>(EItemTableType::General, ItemData.ItemRowName);

        SlotWidgets[TargetSlotIndex]->UpdateSlot(ItemData, CoreData);
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
    
    // 0 ~ EWeaponSlot::Max - 1까지는 장비 전용 슬롯
    for (int32 i = 0; i < MaxSlots; ++i)
    {
        UC_GridItemSlotWidget* NewSlot = CreateWidget<UC_GridItemSlotWidget>(this, SlotWidgetClass);
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
        
        // bHasEquipmentSlots 여부에 따라 장비 슬롯 개수(EWeaponSlot::Max)만큼 오프셋 적용(Max를 사용하면 None이 아무 것도 없는 슬롯이 되서 한칸 버리게 됨)
        int32 StartOffset = InvenComp->GetHasEquipmentSlots() ? static_cast<int32>(EWeaponSlot::None) : 0;
        SetSlotStartIdx(StartOffset);
            
        // 전체 슬롯 다시 그리기
        RefreshAllSlots(InvenComp->GetInventoryItems());
    }
    else
    {
        SetSlotStartIdx(0);
        // 만약 nullptr이 들어왔다면(창고에서 멀어졌다면) UI의 흔적을 청소해 줍니다.
        // (필요에 따라 빈 배열을 넘겨 슬롯을 다 비우거나 비활성화 처리)
        TArray<FInventoryEntry> EmptyArray;
        RefreshAllSlots(EmptyArray);
    }
}

void UC_InventoryGridWidget::SetSlotStartIdx(int32 InSlotStartIdx)
{
    SlotStartIdx = InSlotStartIdx;

    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (SlotWidgets[i])
        {
            // UI의 i번째 칸에게 실제 InvenComponent 상의 인덱스를 재지정 (예: 0 -> 3, 1 -> 4)
            SlotWidgets[i]->SetSlotIndex(i + SlotStartIdx);
        }
    }
}

