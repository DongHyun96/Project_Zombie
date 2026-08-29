// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/C_SkinManager.h"

#include "Skin/C_SkinData.h"
#include "ProjectSettings/C_SkinManagerSettings.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "Utility/C_Util.h"

void UC_SkinManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UC_SkinManagerSettings* Settings = GetDefault<UC_SkinManagerSettings>();

	if (!Settings)
		return;

	if (Settings->SkinDataAssetConfig.IsNull())
		return;

	// 동기 로드
	m_SkinData = Settings->SkinDataAssetConfig.LoadSynchronous();

	if (!m_SkinData)
		return;

	UC_Util::Print("[SkinManager] SkinData load success");
}

void UC_SkinManager::Deinitialize()
{
	m_SkinData = nullptr;

	Super::Deinitialize();
}

//void UC_SkinManager::LoadSkinAsync(EPlayerSkin InSkin, FOnSkinLoaded OnLoaded)
//{
//	if (!m_SkinData)
//		return;
//
//
//}
