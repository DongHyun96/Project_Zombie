// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PoisonAttack.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

UC_PoisonAttack::UC_PoisonAttack()
{
}

void UC_PoisonAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	//UE_LOG(LogTemp, Warning, TEXT("PoisonAttack Activate"));

	_Owner->PlayAnimMontage(_Data->Montage);
}

void UC_PoisonAttack::Fire(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
}

