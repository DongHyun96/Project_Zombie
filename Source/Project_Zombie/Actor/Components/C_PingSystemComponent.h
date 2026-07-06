// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_PingSystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_PingSystemComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	bool TrySpawnPing();
	
private:
	
	class AC_WorldPingActor*	m_WorldPingActor{};
	class UC_CompassBarWidget*	m_CompassBarWidget{}; // CompassBar에 핑 정보 Spawn 시킬 때 필요
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AC_WorldPingActor> m_WorldPingActorClass{};
	
};
