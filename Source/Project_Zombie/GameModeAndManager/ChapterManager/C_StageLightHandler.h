// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_StageLightHandler.generated.h"

enum class EStageTimeOfDay : uint8;

UCLASS()
class PROJECT_ZOMBIE_API AC_StageLightHandler : public AActor
{
	GENERATED_BODY()

public:
	
	AC_StageLightHandler();

protected:
	
	virtual void BeginPlay() override;
	
private:
	
	UPROPERTY()
	TMap<EStageTimeOfDay, class ADirectionalLight*> m_DirectionalLights{};  
	
	UPROPERTY()
	TMap<EStageTimeOfDay, APostProcessVolume*> m_PostProcessVolumes{};

};
