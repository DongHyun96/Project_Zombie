// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_CompassBarWidget.generated.h"

enum class EGamePingType : uint8;
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
	
public:
	
	/// <summary>
	/// 해당 Player의 CompassPingMarker 등록 
	/// </summary>
	/// <returns> : 대응되는 CompassMarkerWidget </returns> 
	class UC_CompassMarkerWidget* RegisterPlayerCompassPingMarker(class AC_BasicPlayer* _Player);
	
public:
	
	/// <summary>
	/// 플레이어가 주인이 아닌 전역적으로 표시할 Ping 보이기 
	/// </summary>
	/// <param name="_GamePingType"></param>
	/// <param name="_WorldPingLocation"></param>
	/// <returns> : 스폰된 CompassMarkerWidget -> 없다면 return nullptr </returns>
	UC_CompassMarkerWidget* SpawnGlobalPingMarker(EGamePingType _GamePingType, const FVector& _WorldPingLocation);

	/// <summary>
	/// 현재 표기중인 모든 CompassPingMarker 감추기 (TODO : 특정 GamePingType만 골라서 지우는 처리가 필요하다면 추가처리가 필요함)
	/// 당장은 일괄적으로 모두 지워버림
	/// </summary>
	void HideAllGlobalPingMarkers();

	/// <summary>
	/// Param으로 넘긴 CompassMarkerWidget 숨김 처리 
	/// </summary>
	void HideGlobalPingMarker(UC_CompassMarkerWidget* _TargetCompassMarkerWidget);
	
protected:

	// 가운데 현재 바라보는 방위 각도 Indicator TextBlock
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DegIndicatorText{}; 
	

protected: // CompassBar 이미지 및 Material 관련

	UPROPERTY(meta = (BindWidget))
	class UImage* CompassBarImage{};

	/* Compass Marker */
	
	// UPROPERTY(meta = (BindWidget))
	// UC_CompassMarkerWidget* CompassPingMarker{}; // TODO : 추후, 최대 인원수만큼 늘려놔야하는 구조로 수정해야 함
	
private:

	// 여기서 CompassPingMarker를 등록하는 Player에 맞게끔 하나씩 빼어서 TMap으로 옮겨담을 것임
	UPROPERTY()
	TArray<UC_CompassMarkerWidget*> m_CompassPingMarkerPool{};
	
	UPROPERTY()
	TMap<class AC_BasicPlayer*, UC_CompassMarkerWidget*> m_mapCompassPingMarkers{};

private: /* Player 주인인 Ping이 아닌, 전역적으로 처리되는 CompassPingMarkerPool */

	UPROPERTY()
	TArray<UC_CompassMarkerWidget*> m_GlobalCompassPingMarkers{};
	
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
	
	const FLinearColor m_ZeroDegColor = FLinearColor(FColor(142, 234, 0, 255));
};
