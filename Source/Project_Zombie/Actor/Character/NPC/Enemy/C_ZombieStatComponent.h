// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ZombieStatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_ZombieStatComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	// 데이터 테이블 포인터
	UPROPERTY(EditAnywhere, Category = "Stat", meta = (RequiredAssetDataTags = "RowStructure=/Script/Project_Zombie.C_ZombieStatData"))
	UDataTable* m_Table;

	// 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FName					m_RowName;

	// 보유 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TMap<FName, float>		m_Stats;

public:
	void AddStat(FName _StatName, float _Amount)
	{
		m_Stats.Add(_StatName, _Amount);
	}

	float GetStat(FName _StatName)
	{
		if (float* pData = m_Stats.Find(_StatName))
		{
			return *pData;
		}
		else
			return 0.f;
	}

	void SetStat(FName _StatName, float _Value)
	{
		if (float* pData = m_Stats.Find(_StatName))
			*pData = _Value;
	}

protected:
	/// 스탯값 가져오기
	void InitStatFromStruct(UScriptStruct* _InStruct, const void* _StrcPtr);

#if WITH_EDITOR
	// 에디터 모드에서만 사용 가능한 가상함수
	// 에디터 상에서 Preview 객체에 변경점이 발생하면 호출되는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& _Event) override;
#endif

protected:
	virtual void BeginPlay() override;

	// 객체가 메모리에 등록될 때 호출(블루 프린트를 열고 닫을 때, 프리뷰 객체가 생성되는 순간)
	virtual void OnRegister() override;

	/// 스탯값 가져오기 초기화
	void InitStat();

public:	
	UC_ZombieStatComponent();
		
};
