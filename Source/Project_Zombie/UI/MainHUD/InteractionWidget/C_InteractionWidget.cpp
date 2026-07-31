// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainHUD/InteractionWidget/C_InteractionWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"

void UC_InteractionWidget::ActivateInteraction(const FText& _InteractionText)
{
	if (!InteractionPanel || !Text_Interaction)
		return;

	// 상호작용 설명 Text 설정
	Text_Interaction->SetText(_InteractionText);

	// 상호작용 UI 활성화
	InteractionPanel->SetVisibility(ESlateVisibility::Visible);
}

void UC_InteractionWidget::DeactivateInteraction()
{
	if (!InteractionPanel)
		return;

	// 상호작용 UI 비활성화
	InteractionPanel->SetVisibility(ESlateVisibility::Collapsed);
}
