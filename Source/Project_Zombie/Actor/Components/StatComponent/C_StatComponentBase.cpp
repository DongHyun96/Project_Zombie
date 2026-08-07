// Fill out your copyright notice in the Description page of Project Settings.


#include "C_StatComponentBase.h"

#include "GlobalData.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PlayerState/C_PlayerState.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"
#include "GlobalData.h"
#include "Actor/Character/Player/C_BasicPlayer.h"


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
	const float InitialMaxHPValue = GetStat(StatName::MaxHP);
	
	//AddStat(TEXT("CurMaxHP"), InitialMaxHPValue);		// 최대 체력
	AddStat(StatName::CurHP, InitialMaxHPValue);		// 현재 체력 또한 Stat 정보에 추가
	
	const float MaxBoost = GetStat(StatName::MaxBoost); // 최대 부스트 게이지 가져오기
	AddStat(StatName::CurBoost, MaxBoost);				// 현재 부스트 게이지 생성하기
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
	
	// 스탯을 추가할 때 해당 스탯의 강화 단계도 추가.
	m_StatGrades.Add(_StatName, 0);
}

float UC_StatComponentBase::GetStat(const FName& _StatName) const
{
	if (const float* pStatValue = m_Stats.Find(_StatName))
		return *pStatValue;
	
	return 0.f;
}

uint8 UC_StatComponentBase::GetStatGrade(const FName& _StatName) const
{
	if (const uint8* pStatGrade = m_StatGrades.Find(_StatName))
		return *pStatGrade;
	
	return 0;
}

void UC_StatComponentBase::SetStat(const FName& _StatName, float _Value)
{
	/*// 클라이언트 환경의 경우, 직접 SetStat 처리를 하는 것이 아닌 서버에 Stat 수정을 요청하여 Multicast로 받는다
	if (!m_OwnerCharacter->HasAuthority())
	{
		Server_SetStat(_StatName, _Value);
		return;
	}
	
	// 서버 환경의 경우, 정확한 SetStat 검증을 통해 서버환경에서의 Stat을 업데이트 -> Multicast로 클라이언트 단에 뿌려주기
	// 실질적인 서버에서의 SetStat 처리가 일어난 상황 -> 이 상황을 클라이언트단에도 맞추기 위해 Multicast를 쏴준다
	if (Local_SetStat(_StatName, _Value))
		Multicast_SetStat(_StatName, _Value);*/
	
	// 결론적으로, 아래의 한 줄로 모든 상황 해결이 된다
	// TODO : 외부의 함수 Call도 Server~ 로 맞출 것(더 직관적 -> 어느 환경에서든, 서버가 Stat을 관리하는 주축이 된다)
	Server_SetStat(_StatName, _Value);
}

void UC_StatComponentBase::IncreaseStatGrade(const FName& _StatName)
{
	Server_IncreaseStatGrade(_StatName);
}

bool UC_StatComponentBase::Local_SetStat(const FName& _StatName, float _Value)
{
	if (_Value < 0.f) return false;

	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;
	
	*pTargetStatValue = _Value;

	// CurHP Set인 경우, Delegate 호출 처리
	if (_StatName == TEXT("CurHP"))
	{
		const float CurMaxHP = GetStat(StatName::MaxHP);
		
		if (CurMaxHP == 0.f) // Divide with 0 error 피하기 위함 (바로 터지기 때문에 처리함)
		{
			PRINT_LOCAL(GetWorld(), "CurMaxHP 0", FColor::Red, 10.f);
			return false;
		}
		
		if (*pTargetStatValue == 0.f)			OnCurHPReachedZeroDelegate.Broadcast(m_OwnerCharacter);
		else if (*pTargetStatValue >= CurMaxHP)	OnCurHPReachedFullDelegate.Broadcast(m_OwnerCharacter);
		
		OnCurHPUpdatedDelegate.Broadcast(*pTargetStatValue / CurMaxHP);
	}
	return true;
}

bool UC_StatComponentBase::Local_IncreaseStatGrade(const FName& _StatName)
{
	//if (_IncreaseAmount < 0.f) return false;
	
	uint8* pTargetStatGrade = m_StatGrades.Find(_StatName);
	if (!pTargetStatGrade) return false;
	
	++(*pTargetStatGrade);
	
	return true;
}

void UC_StatComponentBase::Server_IncreaseStatGrade_Implementation(const FName& _StatName)
{
	if (Local_IncreaseStatGrade(_StatName))
		Multicast_IncreaseStatGrade(_StatName);
}

void UC_StatComponentBase::Multicast_IncreaseStatGrade_Implementation(const FName& _StatName)
{
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_IncreaseStatGrade(_StatName);
}


void UC_StatComponentBase::Server_SetStat_Implementation(const FName& _StatName, float _Value)
{
	// 서버 환경에서의 Stat 맞추기
	// 서버에서의 SetStat이 valid하게 작동되었다면, 나머지 클라이언트 환경에서의 SetStat 처리를 위해 Multicast를 쏴준다
	if (Local_SetStat(_StatName, _Value))
		Multicast_SetStat(_StatName, _Value);
}

void UC_StatComponentBase::Multicast_SetStat_Implementation(const FName& _StatName, float _Value)
{
	// 서버 환경은 이미 최신화 처리 완료 (2번 할 필요 없음)
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_SetStat(_StatName, _Value);
}

void UC_StatComponentBase::IncreaseStat(const FName& _StatName, float _IncreaseAmount)
{
	Server_IncreaseStat(_StatName, _IncreaseAmount);
}

bool UC_StatComponentBase::Local_IncreaseStat(const FName& _StatName, float _IncreaseAmount)
{
	if (_IncreaseAmount < 0.f) return false;
	
	float* pTargetStatValue = m_Stats.Find(_StatName);
	if (!pTargetStatValue) return false;
	
	*pTargetStatValue += _IncreaseAmount;

	// CurHP Set인 경우
	if (_StatName == TEXT("CurHP"))
	{
		// CurHP Full을 찍었으면 해당 Delegate 호출 처리
		const float CurMaxHP = GetStat(StatName::MaxHP);
		if (*pTargetStatValue >= CurMaxHP)
			OnCurHPReachedFullDelegate.Broadcast(m_OwnerCharacter);

		if (CurMaxHP != 0.f)
			OnCurHPUpdatedDelegate.Broadcast(*pTargetStatValue / CurMaxHP);
	}
	
	return true;
}

void UC_StatComponentBase::Server_IncreaseStat_Implementation(const FName& _StatName, float _IncreaseAmount)
{
	if (Local_IncreaseStat(_StatName, _IncreaseAmount))
		Multicast_IncreaseStat(_StatName, _IncreaseAmount);
}

void UC_StatComponentBase::Multicast_IncreaseStat_Implementation(const FName& _StatName, float _IncreaseAmount)
{
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_IncreaseStat(_StatName, _IncreaseAmount);
}

void UC_StatComponentBase::DecreaseStat(const FName& _StatName, float _DecreaseAmount)
{
	if (!m_OwnerCharacter->HasAuthority())
	{
		Server_DecreaseStat(_StatName, _DecreaseAmount);
		return;
	}
	
	if (Local_DecreaseStat(_StatName, _DecreaseAmount))
		Multicast_DecreaseStat(_StatName, _DecreaseAmount);
}

bool UC_StatComponentBase::Local_DecreaseStat(const FName& _StatName, float _DecreaseAmount)
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

		const float CurMaxHP = GetStat(StatName::MaxHP);
		if (CurMaxHP != 0.f)
			OnCurHPUpdatedDelegate.Broadcast(*pTargetStatValue / CurMaxHP);
	}
	
	return true;
}

void UC_StatComponentBase::Server_DecreaseStat_Implementation(const FName& _StatName, float _DecreaseAmount)
{
	if (Local_DecreaseStat(_StatName, _DecreaseAmount))
		Multicast_DecreaseStat(_StatName, _DecreaseAmount);
}

void UC_StatComponentBase::Multicast_DecreaseStat_Implementation(const FName& _StatName, float _DecreaseAmount)
{
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_DecreaseStat(_StatName, _DecreaseAmount);
}

void UC_StatComponentBase::SetCurHP(float _HP)
{
	Server_SetCurHP(_HP);
}

float UC_StatComponentBase::GetCurHP() const
{
	return m_Stats[StatName::CurHP];
}

bool UC_StatComponentBase::Local_SetCurHP(float _HP)
{
	if (_HP > GetStat(StatName::MaxHP)) return false; // 음수 체크는 SetStat에서 처리됨
	return Local_SetStat(StatName::CurHP, _HP); // SetStat에 HP Delegate 들 호출부 포함되어 있음
}

void UC_StatComponentBase::Server_SetCurHP_Implementation(float _HP)
{
	if (Local_SetCurHP(_HP))
		Multicast_SetCurHP(_HP);
}

void UC_StatComponentBase::Multicast_SetCurHP_Implementation(float _HP)
{
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_SetCurHP(_HP);
}

float UC_StatComponentBase::GetCurHPRatio() const
{
	const float CurMaxHPAmount = GetStat(StatName::MaxHP);
	if (CurMaxHPAmount <= 0.f) // 0 나누기 방지
	{
		UC_Util::Print("From UC_StatComponentBase::GetCurHPRatio : Invalid CurMaxHP value", FColor::Red, 10.f);
		return 0.f;
	}
	
	return GetCurHP() / CurMaxHPAmount;
}

void UC_StatComponentBase::IncreaseCurHP(float _IncreaseAmount)
{
	Server_IncreaseCurHP(_IncreaseAmount);
}

bool UC_StatComponentBase::Local_IncreaseCurHP(float _IncreaseAmount)
{
	if (_IncreaseAmount < 0.f) return false;
	
	const float CurMaxHP = GetStat(StatName::MaxHP);

	float* pCurHP = m_Stats.Find(StatName::CurHP);
	if (*pCurHP >= CurMaxHP) return false; // 이미 풀피인 상황이면 Increase 처리 x

	*pCurHP = FMath::Min(*pCurHP + _IncreaseAmount, CurMaxHP);

	OnIncreaseCurHPDelegate.Broadcast(m_OwnerCharacter);
	
	if (*pCurHP >= CurMaxHP) 
		OnCurHPReachedFullDelegate.Broadcast(m_OwnerCharacter);

	if (CurMaxHP != 0.f)
		OnCurHPUpdatedDelegate.Broadcast(*pCurHP / CurMaxHP);
	
	return true;
}

void UC_StatComponentBase::Server_IncreaseCurHP_Implementation(float _IncreaseAmount)
{
	if (Local_IncreaseCurHP(_IncreaseAmount))
		Multicast_IncreaseCurHP(_IncreaseAmount);
}

void UC_StatComponentBase::Multicast_IncreaseCurHP_Implementation(float _IncreaseAmount)
{
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_IncreaseCurHP(_IncreaseAmount);
}

void UC_StatComponentBase::DecreaseCurHP(float _DecreaseAmount)
{
	Server_DecreaseCurHP(_DecreaseAmount);
}

bool UC_StatComponentBase::IsCurHPFull() const
{
	return m_Stats[StatName::CurHP] >= m_Stats[StatName::MaxHP];
}

bool UC_StatComponentBase::IsCurHPZero() const
{
	return m_Stats[StatName::CurHP] <= 0.f;
}

bool UC_StatComponentBase::Local_DecreaseCurHP(float _DecreaseAmount)
{
	if (_DecreaseAmount < 0.f) return false;

	float* pCurHP = m_Stats.Find(StatName::CurHP);
	
	/*if (Cast<AC_BasicPlayer>(m_OwnerCharacter))
		*pCurHP = FMath::Max(1.f, *pCurHP - _DecreaseAmount); // TODO : 다시 0.f로 수정할 것
	else *pCurHP = FMath::Max(0.f, *pCurHP - _DecreaseAmount); // TODO : 다시 0.f로 수정할 것*/
	*pCurHP = FMath::Max(0.f, *pCurHP - _DecreaseAmount);
	
	// CurHP 0을 찍었으면 Delegate 호출 처리
	if (*pCurHP == 0.f) OnCurHPReachedZeroDelegate.Broadcast(m_OwnerCharacter);

	const float CurMaxHP = GetStat(StatName::MaxHP);
	if (CurMaxHP != 0.f)
		OnCurHPUpdatedDelegate.Broadcast(*pCurHP / CurMaxHP);
	
	return true;
}

void UC_StatComponentBase::Server_DecreaseCurHP_Implementation(float _DecreaseAmount)
{
	if (Local_DecreaseCurHP(_DecreaseAmount))
		Multicast_DecreaseCurHP(_DecreaseAmount);
}

void UC_StatComponentBase::Multicast_DecreaseCurHP_Implementation(float _DecreaseAmount)
{
	if (m_OwnerCharacter && m_OwnerCharacter->HasAuthority()) return;
	Local_DecreaseCurHP(_DecreaseAmount);
}