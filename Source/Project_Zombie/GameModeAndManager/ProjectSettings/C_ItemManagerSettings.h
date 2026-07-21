// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "C_ItemManagerSettings.generated.h"

/**
 * 프로젝트 세팅 패널에 노출될 데이터 세팅 전용 클래스
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Item Manager Settings"))
class PROJECT_ZOMBIE_API UC_ItemManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UC_ItemManagerSettings()
	{
		CategoryName = TEXT("Game"); // 프로젝트 세팅의 "Game" 카테고리에 위치
	}

	// TODO : 데이터 테이블 파일들을 나중에 한 곳에 모으고 에디터의 Project Setting에서 Asset Manager의 Primary Asset Type에서 경로 재설정해주기.
	
	// 소모품, 재료 등
	UPROPERTY(Config, EditAnywhere, Category = "DataTables")
	TSoftObjectPtr<UDataTable> GeneralItemDataTableConfig;

	// 총기류
	UPROPERTY(Config, EditAnywhere, Category = "DataTables")
	TSoftObjectPtr<UDataTable> GunDataTableConfig;

	// 근접무기
	UPROPERTY(Config, EditAnywhere, Category = "DataTables")
	TSoftObjectPtr<UDataTable> MeleeDataTableConfig;

	// 투척류
	UPROPERTY(Config, EditAnywhere, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ThrowableDataTableConfig;
};