// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunDataTableComponent.h"
#include "../../../../../GlobalData.h"

UC_GunDataTableComponent::UC_GunDataTableComponent()
{

}

void UC_GunDataTableComponent::BeginPlay()
{
	Super::BeginPlay();

	//InitData();
}

//void UC_GunDataTableComponent::InitData()
//{
//	if (nullptr == m_Table || m_RowName.IsNone())
//		return;
//
//	// 모든 데이터 비움
//	m_Data.Empty();
//
//	// 테이블에 기록된 데이터에 접근
//	FGunData* pGunData = m_Table->FindRow<FGunData>(m_RowName, TEXT("GunData"));
//
//	// 데이터를 구성하고있는 맴버들의 맴버변수명 자체를 키값으로 해서 수치를 기록한다.
//	InitWeaponDataFromStruct(FGunData::StaticStruct(), pGunData);
//
//	// 부모 함수 호출
//	Super::InitData();
//}