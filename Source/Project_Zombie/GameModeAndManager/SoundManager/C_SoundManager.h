// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "C_SoundManager.generated.h"

class USoundMix;
class USoundClass;
class USoundBase;
class UAudioComponent;

UCLASS()
class PROJECT_ZOMBIE_API UC_SoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SetFootstepVolume(float _Volume);

	void PlayBGM();
	void StopBGM();

private:
	UPROPERTY()
	TObjectPtr<USoundMix> m_GameSoundMix;

	UPROPERTY()
	TObjectPtr<USoundClass> m_FootstepSoundClass;

private:
	UPROPERTY()
	UAudioComponent* m_BGMAudioComponent;

};
