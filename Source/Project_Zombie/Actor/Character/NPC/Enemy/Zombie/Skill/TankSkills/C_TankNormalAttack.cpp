// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TankNormalAttack.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

UC_TankNormalAttack::UC_TankNormalAttack()
{
}

bool UC_TankNormalAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	_Owner->PlayAnimMontage(_Data->Montage);


	return true;
}
