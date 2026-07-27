// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_PingSystemComponent.generated.h"


enum class EPingShapeType : uint8;
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
	/// <param name="_Instigator"> : 이 Spawn을 유발한 Object (기록할 필요없다면 nullptr로 둘 것) </param>
	/// <returns> : 제대로 Spawn되지 않았다면 return false </returns>
	bool TrySpawnPing(UObject* _Instigator = nullptr);

	/// <summary>
	/// FullPing 모습 그대로 PingSpawn 처리
	/// </summary>
	/// <param name="_SpawnLocation"> : 핑 스폰 위치 </param>
	/// <param name="_PingType"> : 핑 타입 </param>
	/// <param name="_Instigator"> : 이 Spawn을 유발한 Object (기록할 필요없다면 nullptr로 둘 것)</param>
	void SpawnFullPing(const FVector& _SpawnLocation, EGamePingType _PingType, UObject* _Instigator = nullptr);

	void HidePing();

private:
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnPing
	(
		const FVector&	_SpawnedLocation,
		EGamePingType	_GamePingType,
		EPingShapeType	_PingShapeType
	);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnPing
	(
		const FVector&	_SpawnedLocation,
		EGamePingType	_GamePingType,
		EPingShapeType	_PingShapeType
	);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HidePing();	

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HidePing();
	
public:
	
	// void SetPingColor(const FColor& _PingColor);

public:
	
	UObject* GetLastInstigator() const { return m_LastInstigator; }

private:
	
	class AC_BasicPlayer* m_OwnerPlayer{};
	
private:
	
	class AC_WorldPingActor*	m_WorldPingActor{};
	class UC_CompassBarWidget*	m_CompassBarWidget{}; // CompassBar에 핑 정보 Spawn 시킬 때 필요
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AC_WorldPingActor> m_WorldPingActorClass{};
	
	
private:

	// 마지막 Spawn 주체를 기록 및 판별할 때 사용할 것 
	UPROPERTY()
	UObject* m_LastInstigator{};
	
};
