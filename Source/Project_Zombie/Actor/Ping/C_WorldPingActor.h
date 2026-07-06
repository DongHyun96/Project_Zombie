// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_WorldPingActor.generated.h"

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
	/// <param name="_TraceHitResult"> : 스폰용 LineTrace 검사결과 </param>
	void SpawnPingActorToWorld(const FHitResult& _TraceHitResult);

	void HidePing();
	
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
	
private:
	
	class UC_PingWidget* m_PingWidget{};
	
protected:
	
	UPROPERTY()
	class UNiagaraComponent* m_PingEffectComp{};
	
	
};
