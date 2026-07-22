// Fill out your copyright notice in the Description page of Project Settings.
#include "C_EquipmentWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "UI/InvenUI/C_GridItemSlotWidget.h"
#include "Actor/Components/C_InvenComponent.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "UI/InvenUI/C_EquipmentItemSlotWidget.h"

void UC_EquipmentWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. 슬롯 배열 초기화 및 각 슬롯이 허용할 EItemType 지정
    EquipmentSlots.Empty();

    if (Slot_MainGun)
    {
        Slot_MainGun->SetAllowedItemType(EItemType::MAINWEAPON);
        EquipmentSlots.Add(Slot_MainGun);
    }
    if (Slot_Melee)
    {
        Slot_Melee->SetAllowedItemType(EItemType::MELEEWEAPON);
        EquipmentSlots.Add(Slot_Melee);
    }
    if (Slot_Throwable)
    {
        Slot_Throwable->SetAllowedItemType(EItemType::THROWABLE);
        EquipmentSlots.Add(Slot_Throwable);
    }
    
    /*AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
    if (!Player) return;
    
    UC_InvenComponent* PlayerInvenComponent = Player->GetInvenComponent();
    
    if (!PlayerInvenComponent) return;    
    
    InitEquipmentWidget(PlayerInvenComponent);*/
}

void UC_EquipmentWidget::InitEquipmentWidget(UC_InvenComponent* InInvenComp)
{
    if (!InInvenComp) return;

    if (AssociatedInvenComp)
    {
        AssociatedInvenComp->OnInventorySlotChanged.RemoveDynamic(this, &UC_EquipmentWidget::RefreshEquipmentSlotAt);
    }
    
    AssociatedInvenComp = InInvenComp;
    
    if (AssociatedInvenComp)
    {
        AssociatedInvenComp->OnInventorySlotChanged.RemoveDynamic(this, &UC_EquipmentWidget::RefreshEquipmentSlotAt);
        AssociatedInvenComp->OnInventorySlotChanged.AddDynamic(this, &UC_EquipmentWidget::RefreshEquipmentSlotAt);
    }
    else
    {
        RefreshEquipmentAllSlots();
    }
    
    // 약속된 장비 인덱스 매핑: 0 = MAINWEAPON, 1 = MELEEWEAPON, 2 = THROWABLE
    if (Slot_MainGun)
    {
        Slot_MainGun->SetAssociatedComponent(AssociatedInvenComp);
        Slot_MainGun->SetSlotIndex(0);
    }
    if (Slot_Melee)
    {
        Slot_Melee->SetAssociatedComponent(AssociatedInvenComp);
        Slot_Melee->SetSlotIndex(1);
    }
    if (Slot_Throwable)
    {
        Slot_Throwable->SetAssociatedComponent(AssociatedInvenComp);
        Slot_Throwable->SetSlotIndex(2);
    }

    RefreshEquipmentAllSlots();
}

void UC_EquipmentWidget::RefreshEquipmentAllSlots()
{
    if (AssociatedInvenComp)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        UGameInstance* GI = World->GetGameInstance();
        UC_ItemManager* ItemManager = GI ? GI->GetSubsystem<UC_ItemManager>() : nullptr;
        if (!ItemManager) return;

        //const TArray<FInventoryEntry>& ItemArr = AssociatedInvenComp->GetInventoryItems();
        // AssociatedInvenComp가 유효하면 아이템 배열 참조, 없으면 nullptr
        const TArray<FInventoryEntry>* ItemArrPtr = AssociatedInvenComp ? &AssociatedInvenComp->GetInventoryItems() : nullptr;

        // 2. 모든 장비 슬롯 순회
        for (UC_EquipmentItemSlotWidget* SlotWidget : EquipmentSlots)
        {
            if (!SlotWidget) continue;

            int32 SlotIdx = SlotWidget->GetSlotIndex();

            // 컴포넌트와 데이터 배열이 유효하고, 인덱스가 범위 내에 있을 때
            if (ItemArrPtr && ItemArrPtr->IsValidIndex(SlotIdx))
            {
                const FInventoryEntry& Entry = (*ItemArrPtr)[SlotIdx];
            
                // 아이템 데이터 가져오기 (Entry가 비어있으면 CoreData도 nullptr)
                const FItemData* CoreData = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);
            
                SlotWidget->UpdateSlot(Entry, CoreData);
            }
            else
            {
                // 컴포넌트가 없거나 범위를 벗어난 경우 (빈 슬롯으로 초기화)
                SlotWidget->UpdateSlot(FInventoryEntry(), nullptr);
            }
        }
    }
}

void UC_EquipmentWidget::RefreshEquipmentSlotAt(int32 InIndex, const FInventoryEntry& ItemData)
{
    if (InIndex < 0 || InIndex >= EquipmentSlots.Num()) return;
    
    UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;

    if (EquipmentSlots.IsValidIndex(InIndex) && EquipmentSlots[InIndex])
    {
        const FItemData* CoreData = ItemManager->GetItemData<FItemData>(EItemTableType::General, ItemData.ItemRowName);

        EquipmentSlots[InIndex]->UpdateSlot(ItemData, CoreData);
    }
}
