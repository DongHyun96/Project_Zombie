// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "C_UIManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_UIManager : public AHUD
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlay() override;

public:

	class UC_GameMainHUD* GetMainHUDWidget() const { return m_MainHUDWidget; }
	
protected:

	// HUD로 사용할 최상위 UUserWidget 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> m_MainHUDClass{};
	
protected:

	// 생성된 MainHUD Widget 객체
	UPROPERTY(BlueprintReadOnly)
	UC_GameMainHUD* m_MainHUDWidget{};
	
};
