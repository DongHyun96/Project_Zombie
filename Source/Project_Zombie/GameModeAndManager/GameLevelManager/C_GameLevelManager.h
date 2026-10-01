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

	virtual void Deinitialize() override;
	
public:
	
	void AddPlayer(class AC_BasicPlayer* _Player);
	void RemovePlayer(AC_BasicPlayer* _Player);
	
	const TSet<AC_BasicPlayer*>& GetPlayers() const { return m_Players; }

	AC_BasicPlayer* GetLocalPlayer() const { return m_LocalPlayer; }

	/// <summary>
	/// 모든 플레이어의 상태가 그로기 상태인지
	/// </summary>
	bool HasAllPlayerDead() const;

public:

	class AC_UtilActor* GetUtilActor() const { return m_UtilActor; }
	
private:

	// 이 레벨을 플레이 중인 모든 플레이어 객체
	UPROPERTY()
	TSet<AC_BasicPlayer*> m_Players{};

	UPROPERTY()
	AC_BasicPlayer* m_LocalPlayer{};
	
private:
	
	UPROPERTY()
	AC_UtilActor* m_UtilActor{};
	
};

// 주의 : GetWorld() 가 valid하거나, Valid한 시점에만 사용 & In GameLevel인 경우에만 사용할 것
#define LEVEL_MANAGER GetWorld()->GetSubsystem<UC_GameLevelManager>()

// 현재 Tick에 대응되는 RandomColor를 구할 수 없는 경우 FColor::Red를 반환
#define CUR_TICK_COLOR \
    ((GetWorld() && GetWorld()->GetSubsystem<UC_GameLevelManager>() && \
      GetWorld()->GetSubsystem<UC_GameLevelManager>()->GetUtilActor()) \
        ? GetWorld()->GetSubsystem<UC_GameLevelManager>()->GetUtilActor()->GetCurTickColor() \
        : FColor::Red)