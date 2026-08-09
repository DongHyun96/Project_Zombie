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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:

	UFUNCTION(BlueprintCallable, Category = "SoundManager")
	void SetBGMVolume(float _Volume);

	UFUNCTION(BlueprintCallable, Category = "SoundManager")
	void SetSFXVolume(float _Volume);

private:
	// SoundMix 가 중복 Push 되는것을 방지
	void EnsureSoundMixPushed();

private:
	UPROPERTY()
	TObjectPtr<USoundMix> m_GameSoundMix;

	UPROPERTY()
	TObjectPtr<USoundClass> m_BGMSoundClass;

	UPROPERTY()
	TObjectPtr<USoundClass> m_SFXSoundClass;

	bool m_bSoundMixPushed = false;
};
