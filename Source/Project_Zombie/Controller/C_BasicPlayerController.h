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
	virtual void Destroyed() override;
	
public:
	// 서버 전용: 이 플레이어가 현재 잠그고 있는 인벤토리와 슬롯 인덱스
	UPROPERTY()
	class UC_InvenComponent* Server_ActiveDraggedInven = nullptr;

	UPROPERTY()
	int32 Server_ActiveDraggedSlotIndex = -1;
};
