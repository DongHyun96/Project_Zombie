// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_ItemStatRowWidget.h"

#include "Components/TextBlock.h"
#include "GlobalData.h"

void UC_ItemStatRowWidget::UpdateWidget(EUpgradableStats InStatType, const int32& InCurGrade, const int32& InMaxGrade)
{
	FText name = Helper::GetStatDisplayName(InStatType);
	
	StatName->SetText(name);
	
	CurGrade->SetText(FText::AsNumber(InCurGrade));
	MaxGrade->SetText(FText::AsNumber(InMaxGrade));
}
