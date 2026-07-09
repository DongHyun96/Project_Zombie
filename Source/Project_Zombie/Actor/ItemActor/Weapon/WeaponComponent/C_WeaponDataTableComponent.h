// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_WeaponDataTableComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataValue
{
	GENERATED_BODY()

	// 이 데이터가 Float인지 Asset인지 구분
	UPROPERTY()
	bool					bIsAsset = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (EditCondition = "!bIsAsset", EditConditionHides))
	float					FloatValue = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (EditCondition = "bIsAsset", EditConditionHides))
	TSoftObjectPtr<UObject> AssetValue = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_WeaponDataTableComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	// 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponData")
	FName							m_RowName;

	// 보유 정보 데이터 float
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponData")
	TMap<FName, FWeaponDataValue>	m_Data;


public:
	// Float 데이터 추가 함수
	void AddData(FName _StatName, float _Amount)
	{
		FWeaponDataValue NewValue;
		NewValue.bIsAsset = false;
		NewValue.FloatValue = _Amount;
		NewValue.AssetValue = nullptr;

		m_Data.Add(_StatName, NewValue); // 구조체 형태로 온전하게 전달
	}

	// SoftObjectPtr 데이터 추가 함수
	void AddAssetData(FName _StatName, TSoftObjectPtr<UObject> _Asset)
	{
		FWeaponDataValue NewValue;
		NewValue.bIsAsset = true;
		NewValue.FloatValue = 0.f;
		NewValue.AssetValue = _Asset;

		m_Data.Add(_StatName, NewValue);
	}

	// Float 데이터 Get 함수
	float GetData(FName _StatName)
	{
		// Find는 FWeaponDataValue의 포인터를 반환합니다.
		if (FWeaponDataValue* pData = m_Data.Find(_StatName))
		{
			// 에셋이 아닐 때 float 데이터 반환
			if (!pData->bIsAsset)
			{
				return pData->FloatValue;
			}
		}
		return 0.f;
	}

	// SoftObjectPtr 데이터 Get 함수
	TSoftObjectPtr<UObject> GetAssetData(FName _StatName)
	{
		if (FWeaponDataValue* pData = m_Data.Find(_StatName))
		{
			if (pData->bIsAsset)
			{
				return pData->AssetValue;
			}
		}
		return nullptr;
	}

	// Float 데이터 수정 (사용 할 때 대비)
	void SetData(FName _StatName, float _Value)
	{
		if (FWeaponDataValue* pData = m_Data.Find(_StatName))
		{
			if (!pData->bIsAsset)
			{
				pData->FloatValue = _Value;
			}
		}
	}

	// SoftObjectPtr 데이터 수정  (사용 할 때 대비)
	void SetAssetData(FName _StatName, TSoftObjectPtr<UObject> _Asset)
	{
		if (FWeaponDataValue* pData = m_Data.Find(_StatName))
		{
			if (pData->bIsAsset)
			{
				pData->AssetValue = _Asset;
			}
		}
	}

protected:
	void InitWeaponDataFromStruct(UScriptStruct* _InStruct, const void* _StrctPtr);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& _Event) override;
#endif

protected:
	virtual void BeginPlay() override;

	virtual void InitData();

	virtual void OnRegister() override;

public:
	UC_WeaponDataTableComponent();
		
};
