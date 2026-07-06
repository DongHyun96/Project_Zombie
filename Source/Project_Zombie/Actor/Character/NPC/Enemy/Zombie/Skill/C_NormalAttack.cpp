// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NormalAttack.h"

#include "../../C_BasicEnemy.h"
#include "../../C_EnemySkillData.h"

UC_NormalAttack::UC_NormalAttack()
{
}

void UC_NormalAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	_Owner->PlayAnimMontage(_Data->Montage);
}
