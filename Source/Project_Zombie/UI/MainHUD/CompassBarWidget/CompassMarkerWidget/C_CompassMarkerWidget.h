// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Blueprint/UserWidget.h"
#include "C_CompassMarkerWidget.generated.h"

enum class EPingType : uint8;
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

	void TogglePingMarker(bool _Visible, EGamePingType _PingType = EGamePingType::DefaultMarker);
	void SetWorldMarkerSpawnedLocation(const FVector& _SpawnedLocation) { m_WorldMarkerSpawnedLocation = _SpawnedLocation; }	

	bool IsActive() const { return m_bIsActive; }

private:
	
	void InitDynamicMtrls();
	void UpdateCurrentDynamicMtrl(EGamePingType _PingType);
	
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
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<EGamePingType, UMaterialInstance*>			m_PingMaterialInsts{};

	UPROPERTY(Transient)
	TMap<EGamePingType, UMaterialInstanceDynamic*>	m_CreatedDynamicMtrls{};

	// 현재 사용중인 DynamicMat
private:
	
	UPROPERTY()
	UMaterialInstanceDynamic* m_CurrentDynamicMtrl{};
	
};
