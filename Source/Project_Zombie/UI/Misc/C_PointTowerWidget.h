// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PointTowerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PointTowerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeOnInitialized() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	void SetPercentText(uint8 _Percent);

	/// <summary>
	/// 잠시 색상 빨간색 계열로 변화처리
	/// </summary>
	void OnDamaged();
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ConqueredPercentText{};

private:
	
	FSlateColor m_OriginColor{};
	FSlateColor m_LerpDestColor{};

protected:
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FSlateColor m_DamagedColor{};
	
};
