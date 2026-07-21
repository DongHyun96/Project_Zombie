// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "C_PingWidget.generated.h"

/**
 * World에 배치되는 PingWidget 관리와 동시에, PlayerMainHUD의 CompassBar Ping 정보 노출도 여기서 호출 처리
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	/// <summary>
	/// TODO : Owner Character의 색상에 맞는 Color로 지정해주기  
	/// </summary>
	void SetPingMarkerColor(const FLinearColor& _Color) { PingMarkerImage->SetColorAndOpacity(_Color); }

	void ShowPingWidget(const FVector& _WorldPingSpawnedLocation, EGamePingType _PingType = EGamePingType::DefaultMarker);
	void HidePingWidget();
	
	void SetSpawnedLocation(const FVector& _SpawnedLocation) { m_SpawnedLocation = _SpawnedLocation; }
	
protected:

	UPROPERTY(meta = (BindWidget))
	UImage* PingMarkerImage{};

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DistanceText{};  
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* SpawnAnimation{};
	
private:

	// 현재 Ping 상태가 보여주는 상태인지
	bool m_bCurrentShowingPingMarker{};

	// 현재 플레이 중인 플레이어 (이 핑을 스폰시킨 플레이어가 아닌, 직접 조작중인 플레이어와의 거리를 재야한다)
	class AC_BasicPlayer* m_MyPlayer{};

	// MyPlayer와의 거리를 잴 때, 자신이 spawn된 World 위치를 저장, 거리를 잴 때 사용
	FVector m_SpawnedLocation{};

	// Player HUD CompassBar 내에 위치한 대응되는 CompassMarkerWidget -> 이 친구의 Meter 정보를 여기서 일괄 업데이트 시킴
	UPROPERTY()
	class UC_CompassMarkerWidget* m_TargetCompassMarkerWidget{};

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (DisplayName = "PingMarkerTextures"))
	TMap<EGamePingType, UTexture2D*> m_PingMarkerTextures{};
	
};
