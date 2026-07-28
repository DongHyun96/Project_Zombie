// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_StatComponentBase.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurHPReachedZero, class AC_BasicCharacter*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurHPReachedFull, AC_BasicCharacter*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnIncreaseCurHP, AC_BasicCharacter*);

/// <summary>
/// Param - Ratio
/// </summary>
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurHPUpdated, float);

/*USTRUCT(BlueprintType)
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
	}#1#
	
	bool operator==(const FName& _StatName) const
	{
		return StatName == _StatName; 
	}
};*/

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
	virtual void PostLoad() override;
	
	// 에디터 모드에서만 사용 가능한 가상함수
	// 에디터 상에서 Preview 객체에 변경점이 발생하면 호출되는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& _Event) override;
#endif
	
public:

	void AddStat(const FName& _StatName, float _Amount);
	float GetStat(const FName& _StatName) const;

public: /* 범용적으로 사용 가능한 Stat 처리 관련 함수 */

	/// <returns> : 해당하는 Stat이 없거나 Value가 음수값인 경우(이건 좀 더 따져봐야할듯) </returns>
	bool SetStat(const FName& _StatName, float _Value);

	/// <summary>
	/// 특정 Stat IncreaseAmount 만큼 증가 처리
	/// </summary>
	/// <returns> : 해당하는 Stat이 없거나 Amount가 음수인 경우 return false </returns>
	bool IncreaseStat(const FName& _StatName, float _IncreaseAmount);

	/// <summary>
	/// 특정 Stat DecreaseAmount 만큼 감소 처리 
	/// </summary>
	/// <returns> 해당하는 Stat이 없거나 Amount가 음수인 경우 return false </returns>
	bool DecreaseStat(const FName& _StatName, float _DecreaseAmount);

public: /* 공용 Stat 처리 함수 */
	
	bool SetCurHP(float _HP);
	float GetCurHP() const { return m_Stats[TEXT("CurHP")]; }
	float GetCurHPRatio() const;
	
	bool IncreaseCurHP(float _IncreaseAmount);
	bool DecreaseCurHP(float _DecreaseAmount);
	
	bool IsCurHPFull() const { return m_Stats[TEXT("CurHP")] >= m_Stats[TEXT("CurMaxHP")]; }
	bool IsCurHPZero() const { return m_Stats[TEXT("CurHP")] <= 0.f; }
	
private:
	
	/// <summary>
	/// 공용으로 사용되는 Stat 항목 추가
	/// 자식 StatComponent -> 자신의 항목에 맞는 Stat 항목 추가해줄 것
	/// </summary>
	void InitStat(bool _bModifyForEditor = false);

	/// <summary>
	/// 스탯값 가져오기 
	/// </summary>
	void InitStatFromStruct(UScriptStruct* _InStruct, const void* _StrctPtr);

	/// <summary>
	/// Init시 사용할 DataTable 형식 return (자식 단에서 무조건 구현 처리해줄 것) 
	/// </summary>
	virtual UScriptStruct* GetStatDataStruct() const PURE_VIRTUAL(UC_StatComponentBase::GetStatDataStruct, return nullptr;);
	
	/// <summary>
	/// 자식 StatComponent쪽에서 더 추가할 Stat 내용이 있다면 해당 함수 override하여 추가할 것
	/// </summary>
	virtual void InitAdditionalStat();

protected:
	
	UPROPERTY()
	class AC_BasicCharacter* m_OwnerCharacter{}; // 이 StatComponent를 소유한 OwnerCharacter 
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
	UDataTable* m_Table{};
	
	// 데이터 테이블 행 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat")
	FName m_RowName{};

	// 보유 스탯 (MaxStat, CurStat 모두 포함)
	// UPROPERTY(ReplicatedUsing = OnRep, VisibleAnywhere, BlueprintReadOnly, Category = "Stat") -> 사용 불가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TMap<FName, float> m_Stats{};

public:

	// CurHP 가 모두 소진되었을 때 호출받을 Delegate
	FOnCurHPReachedZero OnCurHPReachedZeroDelegate{};
	
	// CurHP가 100% 회복되었을 때 호출받을 Delegate ( ex) Healer 좀비에서 해당 Delegate 사용 -> 더 이상 힐을 줄 필요가 없다고 판단될 때 쓰임)
	FOnCurHPReachedFull OnCurHPReachedFullDelegate{};
	
	// IncreaseCurHP 정상 처리되었을 시, 호출
	FOnIncreaseCurHP OnIncreaseCurHPDelegate{};

	// HP 가 업데이트 되었을 시, 호출
	FOnCurHPUpdated OnCurHPUpdatedDelegate{};
};
