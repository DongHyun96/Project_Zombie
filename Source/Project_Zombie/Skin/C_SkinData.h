// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalEnum.h"
#include "C_SkinData.generated.h"

class UMaterialInterface;

USTRUCT(BlueprintType)
struct FPlayerSkinData
{
    GENERATED_BODY()

public:
    // 상체 Material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skin")
    TSoftObjectPtr<UMaterialInterface> TopMaterial{};

    // 하체 Material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skin")
    TSoftObjectPtr<UMaterialInterface> BottomMaterial{};
};

UCLASS()
class PROJECT_ZOMBIE_API UC_SkinData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skin")
	TMap<EPlayerSkin, FPlayerSkinData> SkinData{};
};
