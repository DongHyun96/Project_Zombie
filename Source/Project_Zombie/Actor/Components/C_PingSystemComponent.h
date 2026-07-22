// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_PingSystemComponent.generated.h"


enum class EGamePingType : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_PingSystemComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	/// <summary>
	/// 핑 스폰 시도
	/// </summary>
	/// <returns> : 제대로 Spawn되지 않았다면 return false </returns>
	bool TrySpawnPing();

	/// <summary>
	/// FullPing 모습 그대로 PingSpawn 처리
	/// </summary>
	/// <param name="_SpawnLocation"> : 핑 스폰 위치 </param>
	/// <param name="_PingType"> : 핑 타입 </param>
	void SpawnFullPing(const FVector& _SpawnLocation, EGamePingType _PingType);
	
private:
	
	class AC_WorldPingActor*	m_WorldPingActor{};
	class UC_CompassBarWidget*	m_CompassBarWidget{}; // CompassBar에 핑 정보 Spawn 시킬 때 필요
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AC_WorldPingActor> m_WorldPingActorClass{};
	
};
