// Fill out your copyright notice in the Description page of Project Settings.


#include "C_StatComponentBase.h"

#include "GlobalData.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"


UC_StatComponentBase::UC_StatComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_StatComponentBase::OnRegister()
{
	Super::OnRegister();

	// PIE 환경에서 시작 시, OnRegister + BeginPlay 로 인해 InitStat 두 번 호출되는 것 막기 위함 -> 동시에 Blueprint 쪽 내용 업데이트는 안빠지도록 처리
#if WITH_EDITOR
	if (!GetWorld() || !GetWorld()->IsGameWorld())
		InitStat(true);
#endif
}

#if WITH_EDITOR
void UC_StatComponentBase::PostLoad()
{
	Super::PostLoad();

	if (!GetWorld() || !GetWorld()->IsGameWorld())
		InitStat(true);
}

void UC_StatComponentBase::PostEditChangeProperty(FPropertyChangedEvent& _Event)
{
	Super::PostEditChangeProperty(_Event);
	InitStat(true);
}
#endif

void UC_StatComponentBase::BeginPlay()
{
	Super::BeginPlay();

	m_OwnerCharacter = Cast<AC_BasicCharacter>(GetOwner());
	if (!m_OwnerCharacter)
		UC_Util::Print("From UC_StatComponentBase::BeginPlay : Please attach StatComponent to Character based class!", FColor::Red, 10.f);
	
	InitStat();
}

void UC_StatComponentBase::InitStat(bool _bModifyForEditor)
{
	// 테이블과 행 이름이 설정되어 있어야 한다
	if (!m_Table || m_RowName.IsNone())
	{
		if (!_bModifyForEditor) UC_Util::Print("From UC_StatComponentBase::InitStat : m_Table or m_RowName is None!", FColor::Red, 10.f);
		return;
	}

	// ScriptStruct를 자식단에서 제대로 override 하지 않았거나, ExpectedStruct type과 m_Table type이 맞지 않음
	UScriptStruct* ExpectedStruct = GetStatDataStruct();
	if (!ExpectedStruct || m_Table->GetRowStruct() != ExpectedStruct)
	{
		if (!_bModifyForEditor) UC_Util::Print("From UC_StatComponentBase::InitStat : m_Table & ExpectedStruct type mismatched!", FColor::Red, 10.f);
		return;
	}

	// 모든 스탯을 다 지운다
	m_Stats.Empty();

	uint8* RowData = m_Table->FindRowUnchecked(m_RowName);
	if (!RowData)
	{
		if (!_bModifyForEditor) UC_Util::Print("From UC_StatComponentBase::InitStat : m_Table->FindRowUnchecked(m_RowName) failed!", FColor::Red, 10.f);
		return;
	}
	
	// 데이터를 구성하고 있는 멤버들의 멤버변수명 자체를 키값으로 해서 수치를 기록한다.
	InitStatFromStruct(ExpectedStruct, RowData);

	// 추가로 추가할 공용 Stat 추가
	InitAdditionalStat();
	
	if (_bModifyForEditor) Modify();
}

void UC_StatComponentBase::InitAdditionalStat()
{
	const float InitialMaxHPValue = GetStat(TEXT("InitialMaxHP"));
	
	AddStat(TEXT("CurMaxHP"), InitialMaxHPValue);	// 최대 체력
	AddStat(TEXT("CurHP"), InitialMaxHPValue);		// 현제 체력 또한 Stat 정보에 추가
}

void UC_StatComponentBase::InitStatFromStruct(UScriptStruct* _InStruct, const void* _StructPtr)
{
	if (!_InStruct || !_StructPtr) return;

	// 구조체 정보를 순회하면서 멤버 데이터를 확인한다
	for (TFieldIterator<FProperty> Iter(_InStruct); Iter; ++Iter)
	{
		
		FProperty* Property  = *Iter;
		
		// 멤버변수 이름
		const FName StatName = Property->GetFName();

		// Float 타입 멤버만 필터링한다
		FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);

		if (FloatProperty)
		{
			const float Value = FloatProperty->GetPropertyValue_InContainer(_StructPtr);
			AddStat(StatName, Value);
		}
	}
}


void UC_StatComponentBase::AddStat(const FName& _StatName, float _Amount)
{
	// 이미 해당 이름의 스탯이 있으면 추가하지 않는다
	if (m_Stats.Contains(_StatName)) return;
	m_Stats.Add(_StatName, _Amount);
}

float UC_StatComponentBase::GetStat(const FName& _StatName) const
{
	if (const float* pStatValue = m_Stats.Find(_StatName))
		return *pStatValue;
	
	return 0.f;
}

bool UC_StatComponentBase::SetStat(const FName& _StatName, float _Value)
{
	if (_Value < 0.f) return false;

	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;
	
	*pTargetStatValue = _Value;

	// CurHP Set인 경우, Delegate 호출 처리
	if (_StatName == TEXT("CurHP"))
	{
		const float CurMaxHP = GetStat(TEXT("CurMaxHP"));
		
		if (*pTargetStatValue == 0.f)			OnCurHPReachedZeroDelegate.Broadcast(m_OwnerCharacter);
		else if (*pTargetStatValue >= CurMaxHP)	OnCurHPReachedFullDelegate.Broadcast(m_OwnerCharacter);
		
		OnCurHPUpdatedDelegate.Broadcast(*pTargetStatValue / CurMaxHP);
	}
	
	return true;
}

bool UC_StatComponentBase::IncreaseStat(const FName& _StatName, float _IncreaseAmount)
{
	if (_IncreaseAmount < 0.f) return false;
	
	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;
	
	*pTargetStatValue += _IncreaseAmount;

	// CurHP Set인 경우
	if (_StatName == TEXT("CurHP"))
	{
		// CurHP Full을 찍었으면 해당 Delegate 호출 처리
		const float CurMaxHP = GetStat(TEXT("CurMaxHP"));
		if (*pTargetStatValue >= CurMaxHP)
			OnCurHPReachedFullDelegate.Broadcast(m_OwnerCharacter);
		
		OnCurHPUpdatedDelegate.Broadcast(*pTargetStatValue / CurMaxHP);
	}
	
	return true;
}

bool UC_StatComponentBase::DecreaseStat(const FName& _StatName, float _DecreaseAmount)
{
	if (_DecreaseAmount < 0.f) return false;
	
	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;

	*pTargetStatValue = FMath::Max(0.f, *pTargetStatValue - _DecreaseAmount); // 음수값 방지

	// CurHP Set인 경우, CurHP 0을 찍었으면 Delegate 호출 처리
	if (_StatName == TEXT("CurHP"))
	{
		if (*pTargetStatValue <= 0.f)
			OnCurHPReachedZeroDelegate.Broadcast(m_OwnerCharacter);
		
		OnCurHPUpdatedDelegate.Broadcast(*pTargetStatValue / GetStat(TEXT("CurMaxHP")));
	}
	
	return true;
}

bool UC_StatComponentBase::SetCurHP(float _HP)
{
	if (_HP > GetStat("CurMaxHP")) return false; // 음수 체크는 SetStat에서 처리됨
	return SetStat(TEXT("CurHP"), _HP); // SetStat에 HP Delegate 들 호출부 포함되어 있음
}

float UC_StatComponentBase::GetCurHPRatio() const
{
	const float CurMaxHPAmount = GetStat(TEXT("CurMaxHP"));
	if (CurMaxHPAmount <= 0.f) // 0 나누기 방지
	{
		UC_Util::Print("From UC_StatComponentBase::GetCurHPRatio : Invalid CurMaxHP value", FColor::Red, 10.f);
		return 0.f;
	}
	
	return GetCurHP() / CurMaxHPAmount;
}

bool UC_StatComponentBase::IncreaseCurHP(float _IncreaseAmount)
{
	if (_IncreaseAmount < 0.f) return false;
	
	const float CurMaxHP = GetStat(TEXT("CurMaxHP"));

	float* pCurHP = m_Stats.Find(TEXT("CurHP"));
	if (*pCurHP >= CurMaxHP) return false; // 이미 풀피인 상황이면 Increase 처리 x

	*pCurHP = FMath::Min(*pCurHP + _IncreaseAmount, CurMaxHP);

	OnIncreaseCurHPDelegate.Broadcast(m_OwnerCharacter);
	
	if (*pCurHP >= GetStat("CurMaxHP")) 
		OnCurHPReachedFullDelegate.Broadcast(m_OwnerCharacter);
	
	OnCurHPUpdatedDelegate.Broadcast(*pCurHP / CurMaxHP);
	
	return true;
}

bool UC_StatComponentBase::DecreaseCurHP(float _DecreaseAmount)
{
	if (_DecreaseAmount < 0.f) return false;

	float* pCurHP = m_Stats.Find(TEXT("CurHP"));
	*pCurHP       = FMath::Max(1.f, *pCurHP - _DecreaseAmount); // TODO : 다시 0.f로 수정할 것

	// CurHP 0을 찍었으면 Delegate 호출 처리
	if (*pCurHP == 0.f) OnCurHPReachedZeroDelegate.Broadcast(m_OwnerCharacter);
	
	OnCurHPUpdatedDelegate.Broadcast(*pCurHP / GetStat(TEXT("CurMaxHP")));
	
	return true;
}
