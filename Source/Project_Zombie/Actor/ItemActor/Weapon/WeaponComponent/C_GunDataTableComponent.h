// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_GunDataTableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_GunDataTableComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GunData")
	FName				m_RowName;

	// 보유 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GunData")
	TMap<FName, float>	m_GunData;

public:
	/// <summary>
	/// 행이름, 수치 데이터맵에 추가 함수
	/// </summary>
	void AddStat(FName _StatName, float _Amount) 
	{ 
		m_GunData.Add(_StatName, _Amount); 
	}
	
	/// <summary>
	///  함수
	/// </summary>
	float GetStat(FName _StatName)
	{
		if (float* pData = m_GunData.Find(_StatName))
		{
			return *pData;
		}
		else
			return 0.f;
	}

	/// <summary>
	/// 공격(클릭) 시작 시 함수
	/// </summary>
	void SetStat(FName _StatName, float _Value)
	{
		if (float* pData = m_GunData.Find(_StatName))
			*pData = _Value;
	}

protected:
	void InitGunDataFromStruct(UScriptStruct* _InStruct, const void* _StrctPtr);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UC_GunDataTableComponent();

};
