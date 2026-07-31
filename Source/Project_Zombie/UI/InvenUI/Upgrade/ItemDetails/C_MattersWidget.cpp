// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_MattersWidget.h"

#include "GameModeAndManager/C_ItemManager.h"
#include "C_MatterRowWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/ScrollBox.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"
#include "Utility/C_Util.h"


void UC_MattersWidget::UpdateWidget(const FInventoryEntry& InEntry, EUpgradableStats TargetStat)
{
	MattersScrollBox->ClearChildren();
	m_MatterRows.Empty();
	
	if (InEntry.IsEmpty()) return;
	
	const FEquipmentCustomData* EquipmentCustomData = InEntry.GetEquipmentData();
	
	if (!EquipmentCustomData) return;
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	const FItemUpgradeCostRow* UpgradeCostRow = ItemManager->GetWeaponUpgradeCostData(InEntry.ItemRowName);
	
	if (!UpgradeCostRow) return;
	
	const FStatUpgradeCostInfo* CostInfo = UpgradeCostRow->GetTargetStatUpCostInfo(TargetStat);
	
	if (!CostInfo) return;
	
	const uint8 CurGrade = EquipmentCustomData->GetStatGrade(TargetStat);
	
	//if (!CostInfo->GradeCosts.IsValidIndex(CurGrade))
	//{
	//	// 최고 등급 도달 시 처리
	//	return;
	//}
	
	//TArray<FStatUpgradeCostInfo> CostArr = UpgradeCostRow->StatUpgradeCosts;
	
	const FGradeCostInfo& CurrentRecipe = CostInfo->GradeCosts[CurGrade];
	
	UC_InvenComponent* InvenComp = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(PC->GetPawn()))
		{
			InvenComp = Player->GetInvenComponent();
		}
	}
	
	bool HasRequiredItems = true;
	
	for (const FUpgradeMaterialInfo& RequiredCost : CurrentRecipe.RequiredMaterials)
	{
		if (RequiredCost.MatterItemID.IsNone() || RequiredCost.RequiredCount <= 0) continue;

		const FItemData* ItemData = ItemManager->GetItemData<FItemData>(EItemTableType::General, RequiredCost.MatterItemID);
		if (!ItemData) continue;

		if (!MatterRowWidgetClass)
		{
			UC_Util::Print("MatterRowWidgetClass is Not Assigned in BP!", FColor::Red, 5.f);
			return;
		}

		UC_MatterRowWidget* MatterRow = CreateWidget<UC_MatterRowWidget>(this, MatterRowWidgetClass);
		if (MatterRow)
		{
			// 플레이어가 가방에 갖고 있는 해당 재료 수량 계산
			int32 HasCount = InvenComp ? InvenComp->GetTotalItemCount(RequiredCost.MatterItemID) : 0;
		
			if (HasCount < RequiredCost.RequiredCount && ItemUpgradeWidget)
			{
				HasRequiredItems = false;
			}
			
			MatterRow->UpdateWidget(
				ItemData->IconTexture.Get(),
				ItemData->ItemName,
				HasCount,
				RequiredCost.RequiredCount
			);
		
			UC_Util::Print("MatterRowWidget", FColor::Red, 5.f);
			
			MattersScrollBox->AddChild(MatterRow);
			m_MatterRows.Add(MatterRow);
		}
		else
		{
			HasRequiredItems = false;
		}
	}
	
	if (ItemUpgradeWidget)
		ItemUpgradeWidget->SetHasRequiredItems(HasRequiredItems);
}
