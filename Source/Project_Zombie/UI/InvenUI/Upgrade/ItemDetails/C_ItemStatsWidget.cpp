// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_ItemStatsWidget.h"
#include "C_ItemStatRowWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"
#include "Utility/C_Util.h"

void UC_ItemStatsWidget::UpdateWidget(const FEquipmentCustomData* InCustomData)
{
	if (!ItemStatsScrollBox) return;
	
	ItemStatsScrollBox->ClearChildren();
	m_ItemStatRows.Empty();
	if (!InCustomData)
	{
		UC_Util::Print("ItemStatsScrollBox Is Nullptr!");
		return;
	}

	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	//const FEquipmentCustomData& CustomData = InCustomData;
	
	// TODO : 1. 동적 데이터가 가지고 있는 강화 가능 능력치의 배열 기준으로 for문을 돈다. - 구조체를 잘 활용 못하는 느낌.
	// TODO : 2. EUpgradableStats의 최대값(Max)를 이용해서 모든 강화 가능 능력치를 한번씩 돌아 본다. - 강화가능 스탯이 많아지면 불필요한 탐색이 많아짐.
	//const int num =static_cast<int>(EUpgradableStats::Max);
	
	for (int i = 0 ; i < InCustomData->StatList.Num() ; ++i)
	{
		UC_ItemStatRowWidget* ItemStatRow = CreateWidget<UC_ItemStatRowWidget>(this, ItemStatRowWidgetClass);
		
		if (!ItemStatRow)
		{
			UC_Util::Print("ItemStatRowWidget Is Nullptr!");
			continue;
		}
		
		FCustomKeyVal CustomKeyVal = InCustomData->StatList[i];
		
		//const FWeaponData* data = ItemManager->GetWeaponData(ItemUpgradeWidget->GetTargetEntry()->ItemRowName);
		
		//if (!data) continue;
		// TODO : 현재 스탯 가져올 보여 줄 수 있나? 
		
		ItemStatRow->UpdateWidget(CustomKeyVal.Key, CustomKeyVal.Grade, MAX_GRADE); // TODO : Max_Grade 유의
		
		if (CustomKeyVal.Key == ItemUpgradeWidget->GetTargetStat())
			ItemStatRow->GetSelectedRow()->SetVisibility(ESlateVisibility::Visible);
		
		ItemStatsScrollBox->AddChild(ItemStatRow);
		m_ItemStatRows.Add(ItemStatRow);
	}
}

