// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NameTagWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

void UC_NameTagWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	/*if (!m_Visible) return;
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager) return;
		
	UWidgetComponent* WidgetComponent = GetTypedOuter<UWidgetComponent>();
	if (!WidgetComponent) return;

	const FVector WidgetLocation = WidgetComponent->GetComponentLocation();
	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator LookAtRotation = (CameraLocation - WidgetLocation).Rotation();

	WidgetComponent->SetWorldRotation(LookAtRotation);*/
}

void UC_NameTagWidget::ToggleNameTag(bool _Visible)
{
	m_Visible = _Visible;
	SetVisibility(_Visible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UC_NameTagWidget::SetNameTag(const FString& _PlayerName, const FColor& _PlayerColor)
{
	NameText->SetText(FText::FromString(_PlayerName));
	DotImage->SetColorAndOpacity(_PlayerColor);
}
