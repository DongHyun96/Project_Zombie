// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_WorldPingActor.generated.h"

UENUM(BlueprintType)
enum class EGamePingType : uint8
{
	DefaultMarker, // 기본 Marker
	GunBaseMarker,
	AntennaMarker,
	End UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EPingShapeType : uint8
{
	FullPing,			// 다리까지 모두 표시하는 Ping 종류
	IconPing,			// Icon 모양만 표기하는 Ping 종류
	End UMETA(Hidden)
};

/// <summary>
/// World에 배치되는 Ping marker actor
/// </summary>
UCLASS()
class PROJECT_ZOMBIE_API AC_WorldPingActor : public AActor
{
	GENERATED_BODY()

public:
	
	AC_WorldPingActor();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

public:
	
	/// <summary>
	/// Ping 정보 World에 스폰처리
	/// </summary>
	/// <param name="_SpawnLocation"> : Spawn 위치 </param>
	/// <param name="_PingType"> : 핑 종류 </param>
	/// <param name="_PingShapeType"> : 핑 모양 Type </param>
	void SpawnPingActorToWorld
	(
		const FVector&	_SpawnLocation,
		EGamePingType	_PingType,
		EPingShapeType	_PingShapeType
	);
	
	void HidePing();
	
	void SetPingColor(const FColor& _Color);

protected:

	// Ping Marker WidgetComponent (Screen space 사용)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, DisplayName = "PingWidgetComponent")
	class UWidgetComponent* m_PingWidgetComponent{};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, DisplayName = "SplineMeshComponent")
	class USplineMeshComponent* m_SplineMeshComponent{};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, DisplayName = "PingEffect")
	class UNiagaraSystem* m_PingEffect{};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, DisplayName = "RootComp")
	USceneComponent* m_RootSceneComp{};	
	
protected:

	UPROPERTY()
	class UC_PingWidget* m_PingWidget{};
	
protected:
	
	UPROPERTY()
	class UNiagaraComponent* m_PingEffectComp{};
	
	
};
