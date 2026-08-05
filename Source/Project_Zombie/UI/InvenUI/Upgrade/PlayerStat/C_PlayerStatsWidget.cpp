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
		const uint8 StatGrade = StatComp->GetStatGrade(StatName);
		
		// StatRowWidget 생성 후 ScrollBox에 추가
		UC_PlayerStatRowWidget* PlayerStatRow = CreateWidget<UC_PlayerStatRowWidget>(this, PlayerStatRowWidgetClass);
		
		if (!PlayerStatRow)
		{
			UC_Util::Print("ItemStatRowWidget Is Nullptr!");
			continue;
		}
		
		if (PlayerStatRow)
		{
			// TODO : PlayerStat 보여주어야 함. 강화단계를 보여주기 위해서는 
			
			// FName -> FText / FString 변환 후 UI 세팅
			PlayerStatRow->UpdateWidget(StatName, StatValue, StatGrade, MAX_GRADE);
			PlayerStatsScrollBox->AddChild(PlayerStatRow);
			m_PlayerStatRows.Add(PlayerStatRow);
		}
	}
}
