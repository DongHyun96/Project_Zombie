// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemySkillBase.h"

#include "C_EnemySkillData.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "GameModeAndManager/C_UIManager.h"

UC_EnemySkillBase::UC_EnemySkillBase()
{
}

bool UC_EnemySkillBase::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data, OUT int32& _PlayedMontageSectionIdx)
{
	// 최상위 부모에서는 단순 Skill 동작 재생 처리로 Activate 시작 처리
	if (!_Owner || !_Data || !_Data->Montage) return false;

	// 오로지 서버 환경에서만 호출 처리할 것
	if (!_Owner->IsLocallyControlled())
	{
		PRINT_LOCAL(GetWorld(), "From UC_EnemySkillBase::Activate : Call this function only on the server", FColor::Red, 10.f);
		return false;
	}

	// 기본 SkillBase의 경우, 0번 Section Idx 재생
	_PlayedMontageSectionIdx = 0;

	// 정상 재생시작 처리되었다면 Montage Duration float값이 나옴(true) | 재생처리가 되지 않은 경우 0.f (false) )
	const bool Played = static_cast<bool>(_Owner->PlayAnimMontage(_Data->Montage));
	return static_cast<bool>(_Owner->PlayAnimMontage(_Data->Montage));
}

void UC_EnemySkillBase::Fire(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
}

