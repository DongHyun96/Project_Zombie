// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PointIndicatorManagerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PointIndicatorManagerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	/// <summary>
	/// 해당 PointTower의 위치 정보 방면을 표기할 Indicator 등록 
	/// </summary>
	/// <returns> : 이미 동일한 PointTower를 가리키는 Active한 Indicator가 존재한다면 return false, 또는 등록 가능한 Indicator 개수가 부족하다면 return false </returns>
	bool RegisterPointTowerIndicator(class AC_PointTower* _PointTower);

	/// <summary>
	/// 해당 PointTower의 Indicator 등록 해제 
	/// </summary>
	/// <returns> : 등록된 PointTower가 없다면 return false </returns>
	bool DeRegisterPointTowerIndicator(AC_PointTower* _PointTower);
	
private:
	
	/// <summary>
	/// ClampMin, ClampMax 업데이트 -> 처음 초기화한 이후로, 실질적으로 Viewport 크기가 변했을 때 Update 처리를 또 해준다 
	/// </summary>
	void UpdateViewportInfo();

	/// <summary>
	/// WorldLocation이 Clamp 영역에 있는지 확인 & ScreenLocation을 같이 반환 
	/// </summary>
	bool IsWorldLocationWithinScreenClamp(const FVector& _WorldLocation, FVector2D& _OutScreenLocation) const;

	/// <summary>
	/// PointTower가 Screen에 잡히지 않은 때에, Screen의 어느 위치에 Indicator가 위치해야하는지 계산
	/// </summary>
	/// <returns></returns>
	bool CalculateOffScreenLocation(const FVector& _ObjectLocation, class UC_PointIndicatorWidget* _Widget);
	
private:
	
	void UpdateWidgetLocation(UC_PointIndicatorWidget* _Widget, FVector2D _WidgetScreenPos);
	// void UpdateIndicatorAngle(UC_Point)
	void UpdateIndicatorAngle(UC_PointIndicatorWidget* _Widget, const FVector2D& _WidgetScreenPos);
	
private:
	
	UPROPERTY()
	TArray<class UC_PointIndicatorWidget*> m_PointIndicatorWidgetPool{};
	
	UPROPERTY()
	TMap<AC_PointTower*, UC_PointIndicatorWidget*> m_ActivePointIndicators{};
	
	UPROPERTY()
	UC_PointIndicatorWidget* m_IndicatorSample{}; // Size 측정용 (주의 : 이거 직접 조작하지 말 것)

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float m_Accuracy = 2.f;
	
private:

	FVector2D m_SavedViewportSize{};
	FVector2D m_ClampMin{};
	FVector2D m_ClampMax{};
	FVector2D m_ScreenMiddle2D{};

	bool m_bInitializedViewport{};
	bool m_bIndicatorVisible{};
	
};
