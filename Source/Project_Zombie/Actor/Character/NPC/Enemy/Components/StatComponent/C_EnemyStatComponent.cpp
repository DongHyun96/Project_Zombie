// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyStatComponent.h"
#include "C_EnemyStatData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utility/C_Util.h"

UC_EnemyStatComponent::UC_EnemyStatComponent()
{

}

void UC_EnemyStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 최대 속력으로 MaxWalkSpeed 조정
	if (ACharacter* OwnerZombieCharacter = Cast<ACharacter>(GetOwner()))
	{
		const float MoveSpeed = GetStat(TEXT("MoveSpeed"));

		if (MoveSpeed == 0.f)
			UC_Util::Print("From UC_EnemyStatComponent::BeginPlay : " + OwnerZombieCharacter->GetName() + "'s Move Speed is 0.", FColor::Red, 10.f);
		
		OwnerZombieCharacter->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}
	else UC_Util::Print("From UC_EnemyStatComponent::BeginPlay : Owner Actor casting to ACharacter failed!, Please attach this Comp to Zombie class", FColor::Red, 10.f);
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
