// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopGrabMainWeapon.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

bool UC_CopGrabMainWeapon::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	// GrabWeapon 스킬 발동 시, 일단은 해당 Montage 재생 처리를 무조건 통과시킴
	_Owner->PlayAnimMontage(_Data->Montage);
	return true;
}
