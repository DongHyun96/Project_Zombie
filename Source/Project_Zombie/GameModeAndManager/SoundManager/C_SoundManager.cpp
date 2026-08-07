// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/SoundManager/C_SoundManager.h"

void UC_SoundManager::SetFootstepVolume(float _Volume)
{
	if (!m_GameSoundMix || !m_FootstepSoundClass)
		return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;

	const float Volume = FMath::Clamp(_Volume, 0.0f, 1.0f);
	
}
