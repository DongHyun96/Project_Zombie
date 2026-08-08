// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TaskSyncManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "C_GameLevelManager.generated.h"

/**
 * InGame Level Manager 클래스 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_GameLevelManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	
	UC_GameLevelManager();

	/// <summary>
	/// true인 경우, World의 Subsystem으로 추가 
	/// </summary>
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/// <summary>
	/// Level의 가장 첫 BeginPlay로 호출 (레벨에 배치된 Actor들의 BeginPlay 이전에 호출된다)
	/// </summary>
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	
	void AddPlayer(class AC_BasicPlayer* _Player);
	const TArray<AC_BasicPlayer*>& GetPlayers() const { return m_Players; }

	AC_BasicPlayer* GetLocalPlayer() const { return m_LocalPlayer; }

	/// <summary>
	/// 모든 플레이어의 상태가 그로기 상태인지ㄴ 
	/// </summary>
	bool HasAllPlayerDead() const;

private:

	// 이 레벨을 플레이 중인 모든 플레이어 객체
	UPROPERTY()
	TArray<AC_BasicPlayer*> m_Players{};

	UPROPERTY()
	AC_BasicPlayer* m_LocalPlayer{};
	
};

// 주의 : GetWorld() 가 valid하거나, Valid한 시점에만 사용 & In GameLevel인 경우에만 사용할 것
#define LEVEL_MANAGER	GetWorld()->GetSubsystem<UC_GameLevelManager>()
