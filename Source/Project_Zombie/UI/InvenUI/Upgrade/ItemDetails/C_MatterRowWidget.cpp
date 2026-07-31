// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_MatterRowWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UC_MatterRowWidget::UpdateWidget(UTexture2D* InItemIcon, const FText& InItemName, const int32& InHeldCount, const int32& InItemCount)
{
	ItemIcon->SetBrushFromTexture(InItemIcon);
	
	ItemName->SetText(InItemName);
	
	HeldCount->SetText(FText::AsNumber(InHeldCount));
	
	RequiredCount->SetText(FText::AsNumber(InItemCount));
}
