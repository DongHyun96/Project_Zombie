// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/PlayerStat/C_PlayerStatsWidget.h"

#include "C_PlayerStatRowWidget.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/ScrollBox.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"
#include "Utility/C_Util.h"

void UC_PlayerStatsWidget::UpdateWidget(UC_StatComponentBase* StatComp)
{
	if (!PlayerStatsScrollBox || !StatComp) return;
	
	PlayerStatsScrollBox->ClearChildren();
	m_PlayerStatRows.Empty();
	
	const TMap<FName, float>& Stats = StatComp->GetStatsMap();

	
	// TMap 순회 (KeyValueIterator)
	for (const auto& Stat : Stats) 
	{
		const FName& StatName = Stat.Key;
		const float  StatValue = Stat.Value;

		// StatRowWidget 생성 후 ScrollBox에 추가
		UC_PlayerStatRowWidget* PlayerStatRow = CreateWidget<UC_PlayerStatRowWidget>(this, PlayerStatRowWidgetClass);
		
		if (!PlayerStatRow)
		{
			UC_Util::Print("ItemStatRowWidget Is Nullptr!");
			continue;
		}
		
		if (PlayerStatRow)
		{
			// TODO : PlayerStat 보여주어야 함.
			
			// FName -> FText / FString 변환 후 UI 세팅
			//PlayerStatRow->SetStatData(FText::FromName(StatName), StatValue);
			//StatScrollBox->AddChild(RowWidget);
		}
	}
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	
	// TODO : 1. 동적 데이터가 가지고 있는 강화 가능 능력치의 배열 기준으로 for문을 돈다. - 구조체를 잘 활용 못하는 느낌.
	// TODO : 2. EUpgradableStats의 최대값(Max)를 이용해서 모든 강화 가능 능력치를 한번씩 돌아 본다. - 강화가능 스탯이 많아지면 불필요한 탐색이 많아짐.
	//const int num =static_cast<int>(EUpgradableStats::Max);
	
	//for (int i = 0 ; i < EquipCustomData->StatList.Num() ; ++i)
	//{
	//	UC_PlayerStatRowWidget* ItemStatRow = CreateWidget<UC_PlayerStatRowWidget>(this, PlayerStatRowWidgetClass);
	//	
	//	if (!ItemStatRow)
	//	{
	//		UC_Util::Print("ItemStatRowWidget Is Nullptr!");
	//		continue;
	//	}
	//	
	//	FUpgradableKeyVal CustomKeyVal = EquipCustomData->StatList[i];
	//	
	//	//const FWeaponData* data = ItemManager->GetWeaponData(ItemUpgradeWidget->GetTargetEntry()->ItemRowName);
	//	
	//	//if (!data) continue;
	//	// TODO : 현재 스탯 가져와 보여 줄 수 있나?
	//	
	//	ItemStatRow->UpdateWidget(CustomKeyVal.Key, CustomKeyVal.Grade, MAX_GRADE); // TODO : Max_Grade 유의
	//	
	//	if (CustomKeyVal.Key == PlayerStatUpgradeWidget->GetTargetStat())
	//		ItemStatRow->GetSelectedRow()->SetVisibility(ESlateVisibility::Visible);
	//	
	//	PlayerStatsScrollBox->AddChild(ItemStatRow);
	//	m_PlayerStatRows.Add(ItemStatRow);
	//}
}
