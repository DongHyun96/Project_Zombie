// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopFire.h"

#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/C_AIGunUsageComponent.h"
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
	
	if (!Cop->GetEquippedGun()->GetAIGunUsageComponent()->AIFire()) // 발사할 수 없는 상황(총알을 모두 소비하는 등)
	{
		// 스킬 비활성화 처리 (Task쪽 TaskFinished에서 마지막 발사 이후 및 현재 EndSkillManually 처리일 경우,
		// 두 상황 모두 스킬을 끝내고 총을 뱉어내야 하는 상황인지 따져서 던져버림)
		// 일망타진으로 상황조치를 취하는건 OnTaskFinished 쪽에서 구현처리함 (두번 처리 하지 않기 위함)
		Cop->GetSkillComponent()->EndSkillManually();
	}
}
