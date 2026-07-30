// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_SelectedStatWidget.h"

#include "Components/TextBlock.h"

void UC_SelectedStatWidget::UpdateWidget(EUpgradableStats InStatType, const float& InCurStat, const float& InMaxStat)
{
	FText name = Helper::GetStatDisplayName(InStatType);
	
	StatName->SetText(name);
	
	// TODO : 소숫점 제한등이 필요 할 수 있음.
	CurStat->SetText(FText::AsNumber(InCurStat));
	NextStat->SetText(FText::AsNumber(InMaxStat));
}
