// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/SoundManager/C_SoundManager.h"
#include "GameModeAndManager/ProjectSettings/C_SoundManagerSettings.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"


void UC_SoundManager::PlayBGM()
{
	const UC_SoundManagerSettings* Settings = GetDefault<UC_SoundManagerSettings>();
	if (!Settings)
		return;

	USoundBase* BGM = Settings->m_GameBGM.LoadSynchronous();
	if (!BGM)
		return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;

	if (m_BGMAudioComponent)
	{
		m_BGMAudioComponent->Stop();
		m_BGMAudioComponent = nullptr;
	}

	m_BGMAudioComponent = UGameplayStatics::SpawnSound2D(World, BGM);
}

void UC_SoundManager::StopBGM()
{
}

void UC_SoundManager::SetFootstepVolume(float _Volume)
{
	if (!m_GameSoundMix || !m_FootstepSoundClass)
		return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;

	const float Volume = FMath::Clamp(_Volume, 0.0f, 1.0f);
	
}
