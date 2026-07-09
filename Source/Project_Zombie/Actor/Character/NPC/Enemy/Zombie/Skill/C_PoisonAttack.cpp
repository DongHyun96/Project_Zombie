// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PoisonAttack.h"

#include "../../C_BasicEnemy.h"
#include "../../C_EnemySkillData.h"

UC_PoisonAttack::UC_PoisonAttack()
{
}

void UC_PoisonAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	_Owner->PlayAnimMontage(_Data->Montage);
}

