// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_StatComponentBase.generated.h"


USTRUCT(BlueprintType)
struct FStatInfo
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName StatName{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Value{};
	
public:

	/*bool operator==(const FStatInfo& _Other) const
	{
		return StatName == _Other.StatName;
	}*/
	
	bool operator==(const FName& _StatName) const
	{
		return StatName == _StatName; 
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract)
class PROJECT_ZOMBIE_API UC_StatComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_StatComponentBase();

public:
	
	virtual void BeginPlay() override;
	
protected:

	// 객체가 메모리에 등록될 때 호출(블루 프린트를 열고 닫을 때, 프리뷰 객체가 생성되는 순간)
	virtual void OnRegister() override;
	
#if WITH_EDITOR
	// 에디터 모드에서만 사용 가능한 가상함수
	// 에디터 상에서 Preview 객체에 변경점이 발생하면 호출되는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& _Event) override;
#endif
	
public:

	void AddStat(const FName& _StatName, float _Amount);
	float GetStat(const FName& _StatName);
	void SetStat(const FName& _StatName, float _Value);
	
private:
	
	/// <summary>
	/// 공용으로 사용되는 Stat 항목 추가
	/// 자식 StatComponent -> 자신의 항목에 맞는 Stat 항목 추가해줄 것
	/// </summary>
	void InitStat();

	/// <summary>
	/// 스탯값 가져오기 
	/// </summary>
	void InitStatFromStruct(UScriptStruct* _InStruct, const void* _StrctPtr);

	/// <summary>
	/// Init시 사용할 DataTable 형식 return (자식 단에서 무조건 구현 처리해줄 것) 
	/// </summary>
	virtual UScriptStruct* GetStatDataStruct() const PURE_VIRTUAL(UC_StatComponentBase::GetStatDataStruct, return nullptr;)
	
	/// <summary>
	/// 자식 StatComponent쪽에서 더 추가할 Stat 내용이 있다면 해당 함수 override하여 추가할 것
	/// </summary>
	virtual void InitAdditionalStat();
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
	UDataTable* m_Table{};
	
	// 데이터 테이블 행 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat")
	FName m_RowName{};

	// 보유 스탯 (MaxStat, CurStat 모두 포함)
	// UPROPERTY(ReplicatedUsing = OnRep, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TArray<FStatInfo> m_Stats{};

private:
	
};
