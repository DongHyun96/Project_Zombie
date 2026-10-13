// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "C_ChapterManager.generated.h"

UENUM(BlueprintType)
enum class EChapterDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	End UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EChapterTimeOfDay : uint8
{
	Day,
	Night,
	End UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FGameChapterInfo : public FTableRowBase
{
	GENERATED_BODY()

	// Chapter 명칭
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString ChapterName{};

	// 이 Chapter의 Day | Night (Default는 Day)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EChapterTimeOfDay TimeOfDay{};
	
	// 이 Chapter의 난이도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EChapterDifficulty Difficulty{};
	
	// TODO : 썸네일 이미지 정도? -> UI에 띄우는 용도
};

/**
 * 게임 Chapter 상태 관리 -> 전적으로 방장의 Chapter 상태를 모두 따른다
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ChapterManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
	
protected:

	// UPROPERTY(EditDefaultsOnly)
	// TArray<FGameChapterInfo> m_ChapterInfos{};

	// 모든 Chapter 정보 DataTable
	UPROPERTY()
	UDataTable* m_GameChapterInfosData{};
	
		
};
