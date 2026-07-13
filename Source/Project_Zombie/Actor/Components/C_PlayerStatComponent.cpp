// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerStatComponent.h"

#include "GlobalData.h"


UC_PlayerStatComponent::UC_PlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UScriptStruct* UC_PlayerStatComponent::GetStatDataStruct() const
{
	return FPlayerStatData::StaticStruct();
}

/*
void UC_PlayerStatComponent::InitStat()
{
	// 테이블과 행 이름이 설정되어 있어야 한다
	if (!m_Table || m_RowName.IsNone()) return;

	// 모든 스탯을 다 지운다
	m_Stats.Empty();
	
	// 테이블에 기록된 데이터에 접근한다.
	FPlayerStatData* pPlayerStat = m_Table->FindRow<FPlayerStatData>(m_RowName, TEXT("PlayerStat"));

	// 데이터를 구성하고 있는 멤버들의 멤버변수명 자체를 키값으로 해서 수치를 기록한다.
	InitStatFromStruct(FPlayerStatData::StaticStruct(), pPlayerStat);

	// Player만의 추가적인 런타임 스탯 추가
	// AddStat() ...

	// 부모 함수 호출 (공용 스탯 추가)
	Super::InitStat();
}
*/
