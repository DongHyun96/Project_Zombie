// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NormalAttack.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

UC_NormalAttack::UC_NormalAttack()
{
}

bool UC_NormalAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data, OUT int32& _PlayedMontageSectionIdx)
{
	// 재생시킬 공격 Motion이 초기화되어있지 않음
	if (!_Data->Montage) return false;

	const int32 NumSections       = _Data->Montage->CompositeSections.Num();
	const int32 PickedIdx         = FMath::RandRange(0, NumSections - 1);
	const FName PickedSectionName = _Data->Montage->GetSectionName(PickedIdx);

	// Attack 모션 중(섹션 중) 랜덤 Pick된 Section으로 이동해서 재생 처리
	_Owner->PlayAnimMontage(_Data->Montage, 1.f, PickedSectionName);

	_PlayedMontageSectionIdx = PickedIdx;
	
	return true;
}
