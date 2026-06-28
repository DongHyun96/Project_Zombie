#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "Components/UniformGridPanel.h"
#include "C_ItemSlotWidget.h"
#include "GameMode/C_ItemMagnager.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
void UC_InventoryGridWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!ItemGridPanel || !SlotWidgetClass) return;

    // 기존 그리드와 배열 청소
    ItemGridPanel->ClearChildren();
    SlotWidgets.Reset();

    for (int32 i = 0; i < MaxSlots; ++i)
    {
        // 슬롯 위젯 생성
        UC_ItemSlotWidget* NewSlot = CreateWidget<UC_ItemSlotWidget>(this, SlotWidgetClass);
        if (!NewSlot) continue;

        // 슬롯 위젯에게 자신이 몇 번째 칸인지 인덱스를 부여 (드래그 앤 드롭 구현 시 필수)
        NewSlot->SetSlotIndex(i); 

        // 행(Row)과 열(Column) 계산 (이미지의 나누기/나머지 로직)
        int32 CurRow = i / Column;
        int32 CurColumn = i % Column;

        ItemGridPanel->AddChildToUniformGrid(NewSlot, CurRow, CurColumn);

        // 추적 관리를 위해 배열에 보관 
        SlotWidgets.Add(NewSlot);
    }

    APlayerController* PC = GetOwningPlayer();
    if (PC && PC->GetPawn())
    {
        // 프로젝트 플레이어 캐릭터 타입으로 변환
        if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(PC->GetPawn()))
        {
            if (UC_InvenComponent* InvenComp = Player->GetInvenComponent())
            {
                // 델리게이트 중복 방지, TODO : 근데 이 부분이 왜 두번 호출됬는지 확인하고 조치를 취할 것.
                // 두번 호출되진 않는거 같은데 델리게이트 중복 오류가 뜨긴함.
                InvenComp->OnInventorySlotChanged.RemoveDynamic(this, &UC_InventoryGridWidget::RefreshSlotAt);

                // 인벤의 슬롯이 바뀔 때 마다 내 RefreshSlotAt 함수가 정확한 타겟만 찍어서 수행됩니다.
                InvenComp->OnInventorySlotChanged.AddDynamic(this, &UC_InventoryGridWidget::RefreshSlotAt);

                // UI 창이 켜지는 최초 시점에는 도화지가 비어있으므로 한 번 정비해줍니다.
                //RefreshAllSlots(InvenComp->GetInventoryItems());
            }
        }
    }
    //SetVisibility(ESlateVisibility::Visible);
}

void UC_InventoryGridWidget::RefreshAllSlots(const TArray<FInventoryEntry>& InventoryItems)
{
    UC_ItemMagnager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemMagnager>();
    if (!ItemManager) return;

    // 데이터 배열과 위젯 배열의 크기 중 작은 것을 기준으로 안전하게 순회
    int32 LoopCount = FMath::Min(SlotWidgets.Num(), InventoryItems.Num());


    for (int32 i = 0; i < LoopCount; ++i)
    {
        if (!SlotWidgets[i]) continue;

        const FInventoryEntry& Entry = InventoryItems[i];

        // 아이템 매니저를 통해 데이터 테이블의 원본 비주얼/기본 스펙 데이터를 가져옴
        const FItemData* CoreData = ItemManager->GetItemData(Entry.ItemRowName);
        if (!CoreData) continue; // TODO : 인벤에 업는 데이터인 경우 따로 처리해주기.
        // 실시간 인스턴스 데이터(Entry)와 원본 스펙 데이터(CoreData)를 함께 넘겨줌
        SlotWidgets[i]->UpdateSlot(Entry, CoreData);
    }
}

void UC_InventoryGridWidget::RefreshSlotAt(int32 SlotIndex, const FInventoryEntry& ItemData)
{
    UC_ItemMagnager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemMagnager>();
    if (!ItemManager) return;

    // 인덱스 안정성 검사 후 해당 슬롯만 변경
    if (SlotWidgets.IsValidIndex(SlotIndex) && SlotWidgets[SlotIndex])
    {
        const FItemData* CoreData = ItemManager->GetItemData(ItemData.ItemRowName);

        SlotWidgets[SlotIndex]->UpdateSlot(ItemData, CoreData);
    }
}
