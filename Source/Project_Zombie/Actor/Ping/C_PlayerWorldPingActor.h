// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_WorldPingActor.h"
#include "C_PlayerWorldPingActor.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_PlayerWorldPingActor : public AC_WorldPingActor
{
	GENERATED_BODY()

public:
	
	AC_PlayerWorldPingActor();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

private:
	
	UPROPERTY()
	class AC_BasicPlayer* m_OwnerPlayer{};
	
private:
	
	FTimerHandle m_TimerHandle{};
	
};
