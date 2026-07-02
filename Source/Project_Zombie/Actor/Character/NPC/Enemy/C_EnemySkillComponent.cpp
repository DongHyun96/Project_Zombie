// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemySkillComponent.h"

#include "C_EnemySkillBase.h"
#include "C_EnemySkillData.h"

// 비동기 로딩 관련 헤더
#include "Engine/AssetManager.h"

// 팀
#include "GenericTeamAgentInterface.h"

// 키즈멧
#include "Kismet/GameplayStatics.h"

UC_EnemySkillComponent::UC_EnemySkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	for (int32 i = 0; i < (int32)ESkillSlot::END; i++)
	{
		m_SkillSlots.Add(FSkillSlotInfo{ (ESkillSlot)i, });
	}

}


void UC_EnemySkillComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeSkills();
}

void UC_EnemySkillComponent::InitializeSkills()
{
	for (FSkillSlotInfo& Info : m_SkillSlots)
	{
		// DataAsset 로드
		Info.LoadedSkillData = Info.SkillData.LoadSynchronous();

		if (!Info.LoadedSkillData)
			continue;

		// SkillClass 확인
		if (!Info.LoadedSkillData->SkillClass)
			continue;

		// Skill 객체 생성
		Info.SkillInstance = NewObject<UC_EnemySkillBase>(this, Info.LoadedSkillData->SkillClass);
	}
}


void UC_EnemySkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UC_EnemySkillComponent::UseSkill(ESkillSlot _Slot)
{
	if (bUsingSkill)
		return;
	
	// 스킬 사용중으로 설정
	bUsingSkill = true;

	AC_BasicEnemy* Owner = Cast<AC_BasicEnemy>(GetOwner());
	
	if (!Owner)
		return;

	for(FSkillSlotInfo& Info : m_SkillSlots)
	{
		if (Info.SlotType != _Slot)
			continue;

		if (!Info.LoadedSkillData)
			return;

		Info.SkillInstance->Activate(Owner, Info.LoadedSkillData);

		return;
	}

}

