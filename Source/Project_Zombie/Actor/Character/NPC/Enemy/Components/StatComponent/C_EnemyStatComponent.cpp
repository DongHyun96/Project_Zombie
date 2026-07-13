// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyStatComponent.h"
#include "C_EnemyStatData.h"
#include "GlobalData.h"
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

UScriptStruct* UC_EnemyStatComponent::GetStatDataStruct() const
{
	return FEnemyStatData::StaticStruct();
}

/*void UC_EnemyStatComponent::InitStat()
{
	// 테이블과 행 이름이 설정되어 있어야 한다
	if (!m_Table || m_RowName.IsNone()) return;

	// 모든 스탯을 다 지운다
	m_Stats.Empty();
	
	// 테이블에 기록된 데이터에 접근한다.
	FEnemyStatData* pStat = m_Table->FindRow<FEnemyStatData>(m_RowName, TEXT("EnemyStat"));

	// 데이터를 구성하고 있는 멤버들의 멤버변수명 자체를 키값으로 해서 수치를 기록한다.
	InitStatFromStruct(FEnemyStatData::StaticStruct(), pStat);

	// TODO : Enemy만의 추가적인 런타임 스탯 추가 (아직 따로 없음)

	// 부모 함수 호출 (공용 스탯 추가)
	Super::InitStat();
}*/
