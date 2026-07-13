// Fill out your copyright notice in the Description page of Project Settings.


#include "C_StatComponentBase.h"

#include "GlobalData.h"
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

float UC_StatComponentBase::GetStat(const FName& _StatName)
{
	if (float* pStatValue = m_Stats.Find(_StatName))
		return *pStatValue;
	
	return 0.f;
}

bool UC_StatComponentBase::SetStat(const FName& _StatName, float _Value)
{
	if (_Value < 0.f) return false;

	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;
	
	*pTargetStatValue = _Value;
	return true;
	
	/*// 체력 Stat 업데이트의 경우 Delegate 호출 -> 
	if (_StatName == TEXT("CurHP"))
		m_OnTakeDamage.Broadcast(this);*/
}

bool UC_StatComponentBase::IncreaseStat(const FName& _StatName, float _IncreaseAmount)
{
	if (_IncreaseAmount < 0.f) return false;
	
	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;
	
	*pTargetStatValue += _IncreaseAmount;
	return true;
}

bool UC_StatComponentBase::DecreaseStat(const FName& _StatName, float _DecreaseAmount)
{
	if (_DecreaseAmount < 0.f) return false;
	
	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;

	*pTargetStatValue = FMath::Max(0.f, *pTargetStatValue - _DecreaseAmount); // 음수값 방지
	return true;
}

bool UC_StatComponentBase::SetCurHP(float _HP)
{
	if (_HP > GetStat("CurMaxHP")) return false; // 음수 체크는 SetStat에서 처리됨
	return SetStat(TEXT("CurHP"), _HP);
}

bool UC_StatComponentBase::IncreaseCurHP(float _IncreaseAmount)
{
	if (_IncreaseAmount < 0.f) return false;

	float* pCurHP = m_Stats.Find(TEXT("CurHP"));
	*pCurHP       = FMath::Min(*pCurHP + _IncreaseAmount, GetStat("CurMaxHP"));
	
	return true;
}

bool UC_StatComponentBase::DecreaseCurHP(float _DecreaseAmount)
{
	if (_DecreaseAmount < 0.f) return false;

	float* pCurHP = m_Stats.Find(TEXT("CurHP"));
	*pCurHP       = FMath::Max(0.f, *pCurHP - _DecreaseAmount);
	
	return true;
}
