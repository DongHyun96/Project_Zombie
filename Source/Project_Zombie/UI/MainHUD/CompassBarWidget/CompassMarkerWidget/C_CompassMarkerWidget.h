// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_CompassMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_CompassMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:

	void TogglePingMarker(bool _Visible);
	void SetWorldMarkerSpawnedLocation(const FVector& _SpawnedLocation) { m_WorldMarkerSpawnedLocation = _SpawnedLocation; }	

	bool IsActive() const { return m_bIsActive; }

private:
	
	class AC_BasicPlayer*	m_LocalPlayer{};
	APlayerController*		m_LocalPlayerController{};
	
private:
	
	bool m_bIsActive{};

	FVector m_WorldMarkerSpawnedLocation{};
	
protected:

	UPROPERTY(meta = (BindWidget))
	class UImage* CompassMarkerImage{};

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PingMarkerMeterText{};
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Compass")
	float m_CompassHalfWidth{};
	
private:
	
	UMaterialInstanceDynamic* m_MarkerDynamicMtrl{};
	
};
