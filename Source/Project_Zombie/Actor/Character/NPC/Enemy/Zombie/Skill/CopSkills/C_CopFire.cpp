// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopFire.h"

#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
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
	
	// 이 때 실질적인 총기 발사가 이루어져야 함
	if (!Cop || !Cop->GetEquippedGun()) return;
	
	if (!Cop->GetEquippedGun()->AIFire()) // 발사할 수 없는 상황(총알을 모두 소비하는 등)
	{
		// 해당 Skill 비활성화 처리 & 총기 반납 처리해야 함
	}
}
