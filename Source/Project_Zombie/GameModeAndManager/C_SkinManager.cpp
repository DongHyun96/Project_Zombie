// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/C_SkinManager.h"

#include "Skin/C_SkinData.h"
#include "ProjectSettings/C_SkinManagerSettings.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "GlobalEnum.h"

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

void UC_SkinManager::LoadSkinAsync(EPlayerSkin InSkin, FOnSkinLoaded OnLoaded)
{
	if (!m_SkinData)
		return;

	// DataAsset에서 해당 스킨의 Material 정보를 가져옴
	const FPlayerSkinData* SkinData = m_SkinData->SkinData.Find(InSkin);
	if (!SkinData)
		return;

	UMaterialInterface* TopMaterial = SkinData->TopMaterial.Get();
	UMaterialInterface* BottomMaterial = SkinData->BottomMaterial.Get();
	
	// 이미 로드되어 있는 경우, 바로 델리게이트 호출
	if (TopMaterial && BottomMaterial)
	{
		OnLoaded.ExecuteIfBound(TopMaterial, BottomMaterial);
		return;
	}

	// 비동기 로드할 Asset 경로 담아두기
	TArray<FSoftObjectPath> AssetsToLoad;

	if (!TopMaterial && !SkinData->TopMaterial.IsNull())
	{
		AssetsToLoad.Add(SkinData->TopMaterial.ToSoftObjectPath());
	}
	if (!BottomMaterial && !SkinData->BottomMaterial.IsNull())
	{
		AssetsToLoad.Add(SkinData->BottomMaterial.ToSoftObjectPath());
	}

	if (AssetsToLoad.IsEmpty())
		return;


	// 실제 객체 꺼내기 위한 경로
	TSoftObjectPtr<UMaterialInterface> TopMaterialRef = SkinData->TopMaterial;
	TSoftObjectPtr<UMaterialInterface> BottomMaterialRef = SkinData->BottomMaterial;

	// 비동기 로드
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	
	StreamableManager.RequestAsyncLoad(
		AssetsToLoad,
		FStreamableDelegate::CreateWeakLambda(
			this,
			[TopMaterialRef, BottomMaterialRef, OnLoaded]()
			{
				UMaterialInterface* LoadedTop = TopMaterialRef.Get();
				UMaterialInterface* LoadedBottom = BottomMaterialRef.Get();

				if (!LoadedTop || !LoadedBottom)
					return;

				// 로드 완료 후 델리게이트 호출
				OnLoaded.ExecuteIfBound(LoadedTop, LoadedBottom);
			}
		)
	);
}
