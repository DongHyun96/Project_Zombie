// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "C_StageManager.generated.h"

UENUM(BlueprintType)
enum class EStageDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	End UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EStageTimeOfDay : uint8
{
	Day,
	Night,
	End UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FGameStageInfo : public FTableRowBase
{
	GENERATED_BODY()

	// Stage Level명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString StageLevelName{};
	
	// Stage 명칭 (인게임 UI에 표기할)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString StageName{};

	// 이 Stage Day | Night (Default는 Day)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStageTimeOfDay TimeOfDay{};
	
	// 이 Stage 난이도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStageDifficulty Difficulty{};
	
	// TODO : 썸네일 이미지 정도? -> UI에 띄우는 용도
};

/**
 * 게임 Stage 상태 관리 -> 전적으로 방장의 Stage 상태를 모두 따른다
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_StageManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/// <summary>
	/// 현재 StageInfo return | 만약 CurStageIdx가 Valid하지 않다면 return nullptr
	/// </summary>
	const FGameStageInfo* GetCurStageInfo() const;
	
private:

	UPROPERTY()
	UDataTable* m_GameStageInfosData{};
	
	// Clear 여부와 함께 Pair로 묶음 -> 클라는 서버 환경의 ClearedGameStages 상태를 따름
	TArray<TPair<const FGameStageInfo*, bool>> m_ClearedGameStages{};

	// 현재 Stage (직접 고를수도 있게?) -> 미정
	// 역시 클라의 경우 서버 환경의 CurStageIdx 상태를 따름
	uint8 m_CurStageIdx{};
	
};
