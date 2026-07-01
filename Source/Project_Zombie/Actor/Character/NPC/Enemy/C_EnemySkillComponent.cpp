// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemySkillComponent.h"

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

}


void UC_EnemySkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

