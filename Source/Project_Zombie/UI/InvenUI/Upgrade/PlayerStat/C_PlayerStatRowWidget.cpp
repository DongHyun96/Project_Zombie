// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/PlayerStat/C_PlayerStatRowWidget.h"

#include "Components/TextBlock.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"

FReply UC_PlayerStatRowWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		
		AC_UIManager* UIManager = Cast<AC_UIManager>(GetOwningPlayer()->GetHUD());
		
		if (!UIManager) return FReply::Unhandled();

		UIManager->GetInventoryWidget()->GetPlayerStatUpgradeWidget()->SetSelectedStatName(TargetStatName);

		// TODO : MatterRowUpdate해서 보여주기.
		
		//UC_Util::Print(static_cast<int32>(UIManager->GetInventoryWidget()->GetItemUpgradeWidget()->GetTargetStat()));

		// TODO : SelectedStatWidget 보여주기.
		//UIManager->GetInventoryWidget()->GetItemUpgradeWidget()->ShowSelectedStatRow()

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UC_PlayerStatRowWidget::UpdateWidget(const FText& InStatName, const float& InStatValue, const int32& InCurGrade,
                                          const int32& InMaxGrade)
{
	TargetStatName = InStatName;

	//FText name = Helper::GetStatDisplayName(InStatType);
	
	StatName->SetText(TargetStatName);
	
	CurGrade->SetText(FText::AsNumber(InCurGrade));
	MaxGrade->SetText(FText::AsNumber(InMaxGrade));
}
