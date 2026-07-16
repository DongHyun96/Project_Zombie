// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_InventoryWidget.h"

#include "C_InventoryGridWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "DivideWIdget/C_DivideItemWidget.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Utility/C_Util.h"

void UC_InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	UC_InvenComponent* PlayerInvenComponent = Player->GetInvenComponent();

	PlayerGridWidget ->SetParentWidget(this);
	StorageGridWidget->SetParentWidget(this);
	
	PlayerGridWidget->SetInvenComponent(PlayerInvenComponent);
	
	// TODO : 오용되는 부분이 생길 수 있음.
	StorageGridWidget->SetInvenComponent(nullptr);
	
	
}

bool UC_InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                      UDragDropOperation* InOperation)
{
	UC_DragDropOperation* DragDropOperation =  Cast<UC_DragDropOperation>(InOperation);
	
	if (!DragDropOperation) return true;
	
	UC_InvenComponent* invenComp = Cast<AC_BasicPlayer>(GetOwningPlayerPawn())->GetInvenComponent();

	int32 DroppedItemIdx = DragDropOperation->GetSlotIndex();
	
	const FInventoryEntry curSlotItem = invenComp->GetItemAt(DroppedItemIdx);
	
	if (InDragDropEvent.IsControlDown() && curSlotItem.CurCount > 1)
	{
		DivideItemWidget->SetTargetWidget(this);
		ShowDivideItemWidget();
		return true;
	}
	

	
	SpawnItem(DragDropOperation, curSlotItem.CurCount);
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UC_InventoryWidget::ShowDivideEntryWidget()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	FCursorItem cursorItem = Player->GetCurDraggedItem();
	
	if (!cursorItem.bIsValid) return;
	
	DivideItemWidget->SetCursorItem(cursorItem);
	
	DivideItemWidget->ShowDivideEntry();
}

void UC_InventoryWidget::ShowDivideItemWidget()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	FCursorItem cursorItem = Player->GetCurDraggedItem();
	
	if (!cursorItem.bIsValid) return;
	
	DivideItemWidget->SetCursorItem(cursorItem);
	
	DivideItemWidget->ShowDivideItem();
	
}

void UC_InventoryWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);
	
	if (InVisibility == ESlateVisibility::Visible)
	{
		if (StorageGridWidget->GetInvenComponent())
			StorageGridWidget->SetVisibility(InVisibility);
		else
			StorageGridWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}

void UC_InventoryWidget::SpawnItem(UC_DragDropOperation* InDragDropOperation, int32 InCount)
{
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	UC_InvenComponent* invenComp = Cast<AC_BasicPlayer>(GetOwningPlayerPawn())->GetInvenComponent();

	int32 DroppedItemIdx = InDragDropOperation->GetSlotIndex();
	
	FInventoryEntry curSlotItem = invenComp->GetInventoryItems()[DroppedItemIdx];

	UC_ItemManager* ItemMgr = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	// 1. 스폰 위치: 캐릭터 위치보다 약간 위(배나 가슴 높이)에서 스폰해야 자연스럽습니다.
	FVector SpawnLocation = GetOwningPlayerPawn()->GetActorLocation() + FVector(0.f, 0.f, 30.f);
    
	// 2. 던질 방향 벡터 계산 (마인크래프트 스타일)
	FVector ForwardVec = GetOwningPlayerPawn()->GetActorForwardVector();
	FVector UpVec	   = GetOwningPlayerPawn()->GetActorUpVector();
    
	SpawnLocation += ForwardVec * 100;
	// 앞방향으로 300만큼, 윗방향으로 150만큼 힘을 조합 (이 수치는 테스트해보며 조절하세요!)
	FVector LaunchVelocity = (ForwardVec * 300.f) + (UpVec * 150.f);
    
	// 약간의 랜덤성을 부여하면 매번 똑같이 떨어지지 않고 더 자연스러워집니다.
	LaunchVelocity.X += FMath::FRandRange(-50.f, 50.f);
	LaunchVelocity.Y += FMath::FRandRange(-50.f, 50.f);


	
	if (curSlotItem.CurCount == InCount)
	{
		invenComp->InitInvenItemAt(DroppedItemIdx);
	}
	else if (curSlotItem.CurCount > InCount && InCount > 0)
	{
		invenComp->SetEntryCurCount(DroppedItemIdx, curSlotItem.CurCount - InCount);
	}
	else return;

	// 3. 변경된 SpawnItem 호출 (계산한 힘을 넘겨줌)
	ItemMgr->SpawnItem(curSlotItem.ItemRowName, SpawnLocation, InCount, LaunchVelocity);
	
	PlayerGridWidget->RefreshSlotAt(DroppedItemIdx, invenComp->GetInventoryItems()[DroppedItemIdx]);
}
