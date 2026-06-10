// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieStatComponent.h"
#include "C_ZombieStatData.h"

UC_ZombieStatComponent::UC_ZombieStatComponent()
{

}

void UC_ZombieStatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UC_ZombieStatComponent::PostEditChangeProperty(FPropertyChangedEvent& _Event)
{
	Super::PostEditChangeProperty(_Event);

	InitStat();
}


void UC_ZombieStatComponent::OnRegister()
{
	Super::OnRegister();

	InitStat();
}


void UC_ZombieStatComponent::InitStatFromStruct(UScriptStruct* _InStruct, const void* _StrcPtr)
{
	if (nullptr == _InStruct || nullptr == _StrcPtr)
		return;

	for (TFieldIterator<FProperty> Iter(_InStruct); Iter; ++Iter)
	{
		FProperty* Property = *Iter;

		// 멤버변수 이름
		FName StatName = Property->GetFName();

		FFloatProperty* FloatPro = CastField<FFloatProperty>(Property);

		if (FloatPro)
		{
			float Value = FloatPro->GetPropertyValue_InContainer(_StrcPtr);

			AddStat(StatName, Value);
		}
	}
}

void UC_ZombieStatComponent::InitStat()
{
	if (nullptr == m_Table || m_RowName.IsNone())
		return;

	m_Stats.Empty();

	FC_ZombieStatData* pStat = m_Table->FindRow<FC_ZombieStatData>(m_RowName, TEXT("ZombieStat"));

	InitStatFromStruct(FC_ZombieStatData::StaticStruct(), pStat);

	Modify();
}
