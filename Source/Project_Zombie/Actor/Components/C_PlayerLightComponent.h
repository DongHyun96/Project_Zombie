// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_PlayerLightComponent.generated.h"

class AC_BasicPlayer;
class USpotLightComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_PlayerLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UC_PlayerLightComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitLight(USpotLightComponent* _SpotLight);

private:
	UPROPERTY()
	TObjectPtr<AC_BasicPlayer> m_OwnerPlayer;

	UPROPERTY()
	TObjectPtr<USpotLightComponent> m_SpotLight;
};
