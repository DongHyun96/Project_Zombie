// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NurseHealing.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_NurseZombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"

void UC_NurseHealing::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	if (!_Data->Montage) return;

	const int32 NumSections       = _Data->Montage->CompositeSections.Num();
	const int32 PickedIdx         = FMath::RandRange(0, NumSections - 1);
	const FName PickedSectionName = _Data->Montage->GetSectionName(PickedIdx);

	_Owner->PlayAnimMontage(_Data->Montage, 1.f, PickedSectionName);
}

// TODO : 예외적으로 여기서는 Notify 호출로 인한 처리로 들어오지 않음 -> AnimMontage Section의 길이가 모두 달라서 Loop 시에 Heal 구슬 생성 Timeing이 뒤죽박죽 되어버림
// Nurse BT Healing Task에서 Interval 계산해서 Fire 처리될 예정
void UC_NurseHealing::Fire(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	if (!_Owner || !_Data) return;

	AC_NurseZombie* Nurse = Cast<AC_NurseZombie>(_Owner);
	if (!Nurse) return;
	
	const TArray<AC_BasicEnemy*> HealTargets = Nurse->GetHealTargets();

	for (AC_BasicEnemy* HealTarget : HealTargets)
	{
		ZOMBIE_MANAGER->SpawnHealingProjectile
		(
			Nurse->GetActorLocation() + FVector::UnitZ() * 50.f,
			FVector::UnitZ(),
			Nurse,
			HealTarget,
			FMath::RandRange(20.f, 60.f) // 20 ~ 60 범위 힐량의 랜덤한 힐 구슬
		);
	}
}
