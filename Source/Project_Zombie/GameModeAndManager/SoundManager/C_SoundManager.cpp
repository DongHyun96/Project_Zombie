// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/SoundManager/C_SoundManager.h"
#include "GameModeAndManager/ProjectSettings/C_SoundManagerSettings.h"

#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

#include "Kismet/GameplayStatics.h"

void UC_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UC_SoundManagerSettings* Settings = GetDefault<UC_SoundManagerSettings>();
	if (!Settings)
		return;

	m_GameSoundMix = Settings->m_GameSoundMix.LoadSynchronous();
	m_BGMSoundClass = Settings->m_BGMSoundClass.LoadSynchronous();
	m_SFXSoundClass = Settings->m_SFXSoundClass.LoadSynchronous();
}

void UC_SoundManager::Deinitialize()
{
	if (m_bSoundMixPushed)
	{
		UWorld* World = GetGameInstance()->GetWorld();
		if (World && m_GameSoundMix)
		{
			UGameplayStatics::PopSoundMixModifier(World, m_GameSoundMix);
		}
	}

	m_bSoundMixPushed = false;

	m_GameSoundMix = nullptr;
	m_BGMSoundClass = nullptr;
	m_SFXSoundClass = nullptr;

	Super::Deinitialize();
}

void UC_SoundManager::EnsureSoundMixPushed()
{
	// 이미 SoundMix가 Push되어 있다면 return
	if (m_bSoundMixPushed)
		return;

	if (!m_GameSoundMix)
		return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;


	// GameSoundMix를 PushSoundMixModifier로 적용하여 변경 사항을 반영
	UGameplayStatics::PushSoundMixModifier(World, m_GameSoundMix);

	m_bSoundMixPushed = true;
}


void UC_SoundManager::SetBGMVolume(float _Volume)
{
	if (!m_GameSoundMix || !m_BGMSoundClass) 
		return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) 
		return;

	// SoundMix 최초 한 번 활성화
	EnsureSoundMixPushed();

	const float Volume = FMath::Clamp(_Volume, 0.0f, 1.0f);

	UGameplayStatics::SetSoundMixClassOverride(
		World,
		m_GameSoundMix,
		m_BGMSoundClass,
		Volume,
		1.0f, // Pitch
		0.0f, // FadeInTime
		true  // SC_BGM 자식 SoundClass에도 적용
	);
}


void UC_SoundManager::SetSFXVolume(float _Volume)
{
	if (!m_GameSoundMix || !m_SFXSoundClass)
		return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;

	// SoundMix 최초 한 번 활성화
	EnsureSoundMixPushed();

	const float Volume = FMath::Clamp(_Volume, 0.0f, 1.0f);

	UGameplayStatics::SetSoundMixClassOverride(
		World,
		m_GameSoundMix,
		m_SFXSoundClass,
		Volume,
		1.0f,
		0.1f,
		true // SC_SFX 자식 SoundClass에도 적용
	);
}
