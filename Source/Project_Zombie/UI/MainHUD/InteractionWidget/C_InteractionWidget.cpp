// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainHUD/InteractionWidget/C_InteractionWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Materials/MaterialInstanceDynamic.h"


void UC_InteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Image_InteractionTimer)
	{
		m_TimerMaterialInstance = Image_InteractionTimer->GetDynamicMaterial();
	}

	if (TimerPanel)
	{
		TimerPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UC_InteractionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (m_TimerDuration <= 0.0f)
		return;

	m_TimerElapsedTime += InDeltaTime;

	const float Percent = 1.0f - FMath::Clamp(m_TimerElapsedTime / m_TimerDuration, 0.0f, 1.0f);

	if (m_TimerMaterialInstance)
	{
		m_TimerMaterialInstance->SetScalarParameterValue(TEXT("Percent"), Percent);
	}

	if (m_TimerElapsedTime >= m_TimerDuration)
	{
		DeactivateInteractionTimer();
	}
}

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

void UC_InteractionWidget::ActivateInteractionTimer(float _Duration)
{
	if (_Duration <= 0.0f)
	{
		DeactivateInteractionTimer();
		return;
	}

	m_TimerDuration = _Duration;
	m_TimerElapsedTime = 0.0f;

	if (TimerPanel)
	{
		TimerPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (m_TimerMaterialInstance)
	{
		m_TimerMaterialInstance->SetScalarParameterValue(TEXT("Percent"), 1.0f);
	}
}

void UC_InteractionWidget::DeactivateInteractionTimer()
{
	m_TimerDuration = 0.0f;
	m_TimerElapsedTime = 0.0f;

	if (TimerPanel)
	{
		TimerPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}
