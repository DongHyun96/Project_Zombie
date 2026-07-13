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
		InitStat();
#endif
}

void UC_StatComponentBase::PostEditChangeProperty(FPropertyChangedEvent& _Event)
{
	Super::PostEditChangeProperty(_Event);
	InitStat();
}

void UC_StatComponentBase::BeginPlay()
{
	Super::BeginPlay();
	InitStat();
}

void UC_StatComponentBase::InitStat()
{
	// 테이블과 행 이름이 설정되어 있어야 한다
	if (!m_Table || m_RowName.IsNone())
	{
		UC_Util::Print("From UC_StatComponentBase::InitStat : m_Table or m_RowName is None!", FColor::Red, 10.f);
		return;
	}

	// ScriptStruct를 자식단에서 제대로 override 하지 않았거나, ExpectedStruct type과 m_Table type이 맞지 않음
	UScriptStruct* ExpectedStruct = GetStatDataStruct();
	if (!ExpectedStruct || m_Table->GetRowStruct() != ExpectedStruct)
	{
		UC_Util::Print("From UC_StatComponentBase::InitStat : m_Table & ExpectedStruct type mismatched!", FColor::Red, 10.f);
		return;
	}

	// 모든 스탯을 다 지운다
	m_Stats.Empty();

	uint8* RowData = m_Table->FindRowUnchecked(m_RowName);
	if (!RowData)
	{
		UC_Util::Print("From UC_StatComponentBase::InitStat : m_Table->FindRowUnchecked(m_RowName) failed!", FColor::Red, 10.f);
		return;
	}
	
	// 데이터를 구성하고 있는 멤버들의 멤버변수명 자체를 키값으로 해서 수치를 기록한다.
	InitStatFromStruct(FCharacterStatData::StaticStruct(), RowData);

	// 추가로 추가할 공용 Stat 추가
	InitAdditionalStat();
	
	Modify(); // 프리뷰 객체의 변경점을 UE Editor에게 알림, UI의 값을 객체의 값으로 재 반영 강제
}

void UC_StatComponentBase::InitAdditionalStat()
{
	const float InitialMaxHPValue = GetStat("InitialMaxHP");
	if (InitialMaxHPValue)
	{
		AddStat(TEXT("MaxHP"), InitialMaxHPValue); // 최대 체력
		AddStat(TEXT("CurHP"), InitialMaxHPValue); // 현제 체력 또한 Stat 정보에 추가
	}
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
	if (FStatInfo* pInfo = m_Stats.FindByKey(_StatName))
		return;
	
	FStatInfo Info{};

	Info.StatName = _StatName;
	Info.Value    = _Amount;
	
	m_Stats.Add(std::move(Info));
}

float UC_StatComponentBase::GetStat(const FName& _StatName)
{
	if (FStatInfo* pInfo = m_Stats.FindByKey(_StatName))
		return pInfo->Value;
	
	return 0.f;
}

void UC_StatComponentBase::SetStat(const FName& _StatName, float _Value)
{
	if (FStatInfo* pInfo = m_Stats.FindByKey(_StatName))
	{
		pInfo->Value = _Value;

		/*// 체력 Stat 업데이트의 경우 Delegate 호출
		if (_StatName == TEXT("CurHP"))
			m_OnTakeDamage.Broadcast(this);*/
	}
}
