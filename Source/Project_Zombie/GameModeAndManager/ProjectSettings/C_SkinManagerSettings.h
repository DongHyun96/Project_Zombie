// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "C_SkinManagerSettings.generated.h"

class UC_SkinData;

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Skin Manager Settings"))

class PROJECT_ZOMBIE_API UC_SkinManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UC_SkinManagerSettings()
	{
		CategoryName = TEXT("Game"); // 프로젝트 세팅의 "Game" 카테고리에 위치
	}

public:
	// 스킨 데이터 테이블
	UPROPERTY(Config, EditAnywhere, Category = "Skin")
	TSoftObjectPtr<UC_SkinData> SkinDataAssetConfig{};
};
