// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemySkillComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillBase.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

// 비동기 로딩 관련 헤더
#include "Engine/AssetManager.h"

// 팀
#include "GenericTeamAgentInterface.h"

// 키즈멧
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

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

	// 주인 캐릭터 초기화
	m_OwnerCharacter = Cast<AC_BasicCharacter>(GetOwner());
	if (!m_OwnerCharacter)
	{
		// 캐릭터 Base인 객체만 일단 Skill 프레임워크를 사용할 수 있다고 가정
		UC_Util::Print("From UC_EnemySkillComponent::BeginPlay : Pleash attach SkillComponent to Character based class!", FColor::Red, 10.f);
	}
	
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

bool UC_EnemySkillComponent::UseSkill(ESkillSlot _Slot)
{
	// 이미 다른 스킬을 사용중일 때
	if (bUsingSkill) return false;
	
	UE_LOG(LogTemp, Warning, TEXT("UseSkill"));

	AC_BasicEnemy* Owner = Cast<AC_BasicEnemy>(GetOwner());
	if (!Owner) return false;

	const uint8 TargetSlotIdx  = static_cast<uint8>(_Slot);
	const FSkillSlotInfo& Info = m_SkillSlots[TargetSlotIdx];
	
	if (!Info.LoadedSkillData)
	{
		UC_Util::Print("From UC_EnemySkillComponent::UseSkill : TargetSlot skill not loaded!", FColor::Red, 10.f);
		return false;
	}

	// 현재 해당 스킬을 사용할 수 없는 상황
	if (!Info.SkillInstance->Activate(Owner, Info.LoadedSkillData))
		return false;

	// 스킬 사용 성공

	m_CurSkillData = Info.LoadedSkillData;	// 현재 사용중인 Skill 세팅
	bUsingSkill    = true;					// 스킬 사용중으로 설정
	return true;
}

void UC_EnemySkillComponent::OnAN_EndSkill()
{
	bUsingSkill = false;

	m_SkillEndDelegate.Broadcast(Cast<AC_BasicEnemy>(GetOwner()));

	m_CurSkillData = nullptr;

}

void UC_EnemySkillComponent::EndSkillManually()
{
	// AnimNotify를 통해(AnimMontage 종료시점을 통해) 호출된 것이 아니기 때문에 Montage 모션을 직접 Stop 시켜주어야 한다.
	// 현재 Skill이 Valid하고, 해당 Skill의 모션이 끝나지 않았다면 끊어줌
	if (m_CurSkillData && m_OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(m_CurSkillData->Montage))
		m_OwnerCharacter->StopAnimMontage(m_CurSkillData->Montage);

	// 나머지 처리는 기존의 OnEndSkill 처리와 같음
	OnAN_EndSkill();
}

void UC_EnemySkillComponent::Fire()
{
	if (!m_CurSkillData)
		return;

	for (FSkillSlotInfo& Info : m_SkillSlots)
	{
		if (Info.LoadedSkillData == m_CurSkillData)
		{
			Info.SkillInstance->Fire(Cast<AC_BasicEnemy>(GetOwner()), Info.LoadedSkillData);
			return;
		}
	}
}

float UC_EnemySkillComponent::GetSkillRange(ESkillSlot _Slot) const
{
	for (const FSkillSlotInfo& Info : m_SkillSlots)
	{
		if (Info.SlotType == _Slot && Info.LoadedSkillData)
		{
			return Info.LoadedSkillData->Range;
		}
	}

	return 0.f;
}

