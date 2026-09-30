// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PointIndicatorWidget.generated.h"

/**
 * 현재 Sequence의 PointTower 방면 정보 가리키기용 Widget
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PointIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:

	void SetPointTower(class AC_PointTower* _PointTower);
	
	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	
	bool IsCurrentlyVisible() const { return m_bVisible; }
	
	void SetTriangleImgAngle(float _Angle);
	
private:

	// 가리킬 PointTower, 만약 가리키는 PointTower가 존재하지 않다면 기능하지 않는다
	UPROPERTY()
	AC_PointTower* m_TargetPointTower{};
	
	bool m_bVisible{};
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	class UImage* TriangleImage{};
	
};
