// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_ItemStatRowWidget.h"

#include "Components/TextBlock.h"
#include "GlobalData.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"
#include "Utility/C_Util.h"
FReply UC_ItemStatRowWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		
		AC_UIManager* UIManager = Cast<AC_UIManager>(GetOwningPlayer()->GetHUD());
		
		if (!UIManager) return FReply::Unhandled();

		UIManager->GetInventoryWidget()->GetItemUpgradeWidget()->SetTargetStat(TargetStat);

		UC_Util::Print(static_cast<int32>(UIManager->GetInventoryWidget()->GetItemUpgradeWidget()->GetTargetStat()));

		// TODO : SelectedStatWidget 보여주기.

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UC_ItemStatRowWidget::UpdateWidget(EUpgradableStats InStatType, const int32& InCurGrade, const int32& InMaxGrade)
{
	TargetStat = InStatType;

	FText name = Helper::GetStatDisplayName(InStatType);
	
	StatName->SetText(name);
	
	CurGrade->SetText(FText::AsNumber(InCurGrade));
	MaxGrade->SetText(FText::AsNumber(InMaxGrade));
}
