// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "C_BasicPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_BasicPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void OnUnPossess() override;
	
public:
	
	void FinishItemUpgrade();
	
	UFUNCTION(Client, Reliable)
	void Client_FinishItemUpgrade();
	
	void FinishPlayerStatUpgrade();

	// 멀티 캐스트로 바꾸고 IsLocallyController사용하기.
	UFUNCTION(Client, Reliable)
	void Client_FinishPlayerStatUpgrade();
	
	UFUNCTION()
	void OnRep_IsUpgradingItem();
	
	UFUNCTION()
	void OnRep_IsUpgradingPlayerStat();
public:
	void SetIsUpgradingItem(bool InIsUpgrading) { bIsUpgradingItem = InIsUpgrading; }
	
	bool GetIsUpgradingItem() { return bIsUpgradingItem; }
	
	void SetIsUpgradingPlayerStat(bool InIsUpgrading) { bIsUpgradingPlayerStat = InIsUpgrading; }
	
	bool GetIsUpgradingPlayerStat() { return bIsUpgradingPlayerStat; }

protected:
	virtual void Destroyed() override;
	
	// 리플리케이트 할 변수 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	// 서버 전용: 이 플레이어가 현재 잠그고 있는 인벤토리와 슬롯 인덱스
	// TODO : 여기서 CursorItem을 사용할까?
	UPROPERTY()
	class UC_InvenComponent* Server_ActiveDraggedInven = nullptr;

	UPROPERTY()
	int32 Server_ActiveDraggedSlotIndex = -1;
	
protected:

	// 서버에서 현재 Item을 강화중인지 판단하는 함수. 
	UPROPERTY(ReplicatedUsing = OnRep_IsUpgradingItem)
	bool bIsUpgradingItem = false;
	
	// 서버에서 현재 Player Stat을 강화중인지 판단하는 함수. 
	UPROPERTY(ReplicatedUsing = OnRep_IsUpgradingPlayerStat)
	bool bIsUpgradingPlayerStat = false;
};
