// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyStatComponent.h"
#include "C_EnemyStatData.h"

UC_EnemyStatComponent::UC_EnemyStatComponent()
{

}

void UC_EnemyStatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UC_EnemyStatComponent::InitStatFromStruct(UScriptStruct* _InStruct, const void* _StrcPtr)
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

void UC_EnemyStatComponent::PostEditChangeProperty(FPropertyChangedEvent& _Event)
{
	Super::PostEditChangeProperty(_Event);

	InitStat();
}


void UC_EnemyStatComponent::OnRegister()
{
	Super::OnRegister();

	InitStat();
}


void UC_EnemyStatComponent::InitStat()
{
	if (nullptr == m_Table || m_RowName.IsNone())
		return;

	m_Stats.Empty();

	FC_EnemyStatData* pStat = m_Table->FindRow<FC_EnemyStatData>(m_RowName, TEXT("ZombieStat"));

	InitStatFromStruct(FC_EnemyStatData::StaticStruct(), pStat);

	Modify();
}
