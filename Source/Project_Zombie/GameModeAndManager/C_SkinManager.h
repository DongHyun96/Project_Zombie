// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "C_SkinManager.generated.h"

class UC_SkinData;
class UMaterialInterface;

// Material 비동기 로드 완료 시 호출되는 델리게이트
DECLARE_DELEGATE_TwoParams(FOnSkinLoaded, UMaterialInterface*, UMaterialInterface*);

UCLASS()
class PROJECT_ZOMBIE_API UC_SkinManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// 스킨 Material 비동기 로드
	//void LoadSkinAsync(EPlayerSkin InSkin, FOnSkinLoaded OnLoaded);

private:
	// 동기 로드 완료된 데이터 에셋 포인터 (런타임 캐싱)
	UPROPERTY()
	TObjectPtr<UC_SkinData> m_SkinData = nullptr;
};
