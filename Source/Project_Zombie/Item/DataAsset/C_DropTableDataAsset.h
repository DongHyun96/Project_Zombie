// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Engine/DataAsset.h"
#include "C_DropTableDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_DropTableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// AssetManager에서 에셋을 식별할 수 있도록 Id 반환 함수 오버라이드
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		// "DropTable"이라는 타입 이름과 에셋의 고유 FName을 짝지어 반환
		return FPrimaryAssetId("DropTable", GetFName());
	}
	
public:
	// 한 마리가 토해낼 수 있는 최대 아이템 수 (대량 좀비전 성능 방어선)
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limit")
	//int32 MaxDropCountPerMonster = 3;

	// 독립 확률 드랍 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Entries")
	TArray<FDropEntry> DropEntries;
	
	// 아이템이 몬스터 중심에서 사방으로 퍼질 반경 (기본값: 50cm ~ 100cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	float m_DropScatterRadius = 50.f;
};
