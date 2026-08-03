// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ItemDetails/C_ItemStatRowWidget.h"
#include "ItemDetails/C_ItemStatsWidget.h"
#include "PlayerStat/C_PlayerStatRowWidget.h"
#include "PlayerStat/C_PlayerStatsWidget.h"
#include "PlayerStat/C_PlayerUpMattersWidget.h"

void UC_PlayerStatUpgradeWidget::SetSelectedStatName(const FText& InSelectedStatName)
{
	SelectedStatName = InSelectedStatName;
	
	if (SelectedStatName.IsEmpty()) return;
	
	// TODO(상연) : 원래 이런건 여기서 구현하는게 맞나? 상호작용이 일어난 StatRowWidget쪽에서 했어야 하는건 아닐까?
	
	TArray<UC_PlayerStatRowWidget*> StatRowArr = PlayerStatsWidget->GetItemStatRows();
	
	for (int i = 0; i < StatRowArr.Num(); ++i)
	{
		if (StatRowArr[i]->GetTargetStat().EqualTo(SelectedStatName))
			StatRowArr[i]->GetSelectedRow()->SetVisibility(ESlateVisibility::Visible);
		else
			StatRowArr[i]->GetSelectedRow()->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	PlayerUpMattersWidget->UpdateWidget(m_UsePlayer);
}

void UC_PlayerStatUpgradeWidget::InitWidget()
{
	PlayerIcon->SetVisibility(ESlateVisibility::Collapsed);
	// TODO : 여기 들어오면 위젯들 초기화 해주기.
	PlayerName->SetText(FText());
	//PlayerStatsWidget->UpdateWidget(FInventoryEntry());
	
	m_UsePlayer = nullptr;
	SelectedStatName = FText();
	
	bIsUpgrading = false;	
	hasRequiredItems = false;
}

void UC_PlayerStatUpgradeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	PlayerStatsWidget->SetParentWidget(this);
	PlayerUpMattersWidget->SetParentWidget(this);
}
