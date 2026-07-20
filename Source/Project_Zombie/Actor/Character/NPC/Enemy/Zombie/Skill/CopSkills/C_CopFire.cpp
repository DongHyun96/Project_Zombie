// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopFire.h"

#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Utility/C_Util.h"

bool UC_CopFire::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	// TODO : 일단 당장에는 Rifle Section만 사용(바로 재생 처리)
	// -> 추후 무기 종류가 늘어나면, 해당하는 무기의 Fire동작 Section을 재생처리 시켜주어야 함
	return Super::Activate(_Owner, _Data);
}

void UC_CopFire::Fire(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	AC_CopZombie* Cop = Cast<AC_CopZombie>(_Owner);
	// if (!Cop || !Cop->GetEquippedGun()) return;
}
