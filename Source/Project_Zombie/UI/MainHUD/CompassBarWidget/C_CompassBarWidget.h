// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_CompassBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_CompassBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	
	void SetDisplayDegreeText(float _RotationYaw);

protected:

	// 가운데 현재 바라보는 방위 각도 Indicator TextBlock
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DegIndicatorText{}; 
	

protected: // CompassBar 이미지 및 Material 관련

	UPROPERTY(meta = (BindWidget))
	class UImage* CompassBarImage{};
	
private:
	
	UMaterialInstanceDynamic* m_CompassBarDynamicMtrl{};
	
private:
	
	const TMap<int32, FText> m_DegreeToAlphabet = 
	{
		{315, 	FText::FromString(TEXT("NW"))},
		{270, 	FText::FromString(TEXT("W"))},
		{225, 	FText::FromString(TEXT("SW"))},
		{180, 	FText::FromString(TEXT("S"))},
		{135, 	FText::FromString(TEXT("SE"))},
		{90,	FText::FromString(TEXT("E"))},
		{45,	FText::FromString(TEXT("NE"))},
		{0,		FText::FromString(TEXT("N"))},
	};
	
	const FLinearColor m_ZeroDegColor = FLinearColor(FColor(255, 220, 0, 255));
};
