// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "C_SoundManagerSettings.generated.h"

class USoundBase;
class USoundMix;
class USoundClass;

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Sound Manager Settings"))
class PROJECT_ZOMBIE_API UC_SoundManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UC_SoundManagerSettings()
	{
		CategoryName = TEXT("Game"); // 프로젝트 세팅의 "Game" 카테고리에 위치
	}

public:

	UPROPERTY(Config, EditAnywhere, Category = "SoundMix")
	TSoftObjectPtr<USoundMix> m_GameSoundMix;

	UPROPERTY(Config, EditAnywhere, Category = "SoundClass")
	TSoftObjectPtr<USoundClass> m_BGMSoundClass;

	UPROPERTY(Config, EditAnywhere, Category = "SoundClass")
	TSoftObjectPtr<USoundClass> m_SFXSoundClass;

	// 우선순위 만들때 필요
	//UPROPERTY(Config, EditAnywhere, Category = "SoundClass")
	//TSoftObjectPtr<USoundClass> m_FootstepSoundClass;
};
