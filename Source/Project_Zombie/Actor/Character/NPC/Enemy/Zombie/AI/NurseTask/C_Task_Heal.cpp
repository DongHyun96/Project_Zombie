// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_Heal.h"

#include <string>

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_NurseZombie.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Chaos/ChaosVDTraceRelayTransport.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

UC_Task_Heal::UC_Task_Heal()
{
	bCreateNodeInstance = false;
	m_SkillSlot = ESkillSlot::Skill_2; // Nurse의 경우 SkillComponent의 Slot 2번이 힐 스킬로 설정되어 있음
	
	bNotifyTick = true;
}

EBTNodeResult::Type UC_Task_Heal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Activate Skill 실패
	if (Super::ExecuteTask(OwnerComp, NodeMemory) == EBTNodeResult::Failed)
		return EBTNodeResult::Failed;

	// Activate Skill 성공
	// TickTask에서 시간 재는 Timer 노드 메모리 초기화
	*reinterpret_cast<float*>(NodeMemory) = 0.f;
	
	AC_NurseZombie* NurseZombie = Cast<AC_NurseZombie>(OwnerComp.GetAIOwner()->GetPawn());
	NurseZombie->ToggleHealingAura(true);
	
	return EBTNodeResult::InProgress;
}

void UC_Task_Heal::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
	Super::TickTask(_OwnCom, _NodeMemory, _DeltaSeconds);
	
	static const float HEAL_PROJECTILE_SPAWN_INTERVAL = 1.5f;

	AC_NurseZombie* Nurse = Cast<AC_NurseZombie>(_OwnCom.GetAIOwner()->GetPawn());
	
	if (!Nurse) // 여기 애초에 들어오면 안됨
	{
		FinishLatentTask(_OwnCom, EBTNodeResult::Failed);
		return;
	}

	// 더 이상 힐을 줄 Target이 존재하지 않음
	if (Nurse->GetHealProjectileTargets().IsEmpty())
	{
		Nurse->GetSkillComponent()->EndSkillManually();
		return;
	}

	/* Heal Aura 처리 관련 */
	TArray<AActor*> HealAuraOverlappingEnemies{};
	Nurse->GetHealingAuraOverlappingEnemies(HealAuraOverlappingEnemies);

	for (AActor* Actor : HealAuraOverlappingEnemies)
	{
		AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(Actor);
		if (!Enemy) continue;

		// Heal Aura에 들어온 Enemy에 대해 초당 힐 부여
		const float CurrentHealAmount = _DeltaSeconds * Nurse->GetHealingAuraHPS();
		Enemy->GetStatComponent()->IncreaseCurHP(CurrentHealAmount);
	}
	
	/* Healing Projectile 처리 관련 */
	float* ProjectileSpawnIntervalTimer  = reinterpret_cast<float*>(_NodeMemory);
	*ProjectileSpawnIntervalTimer       += _DeltaSeconds;

	// Projectile 스폰 시간 다 됨 -> Target들에게 힐 Projectile 보내기
	if (*ProjectileSpawnIntervalTimer > HEAL_PROJECTILE_SPAWN_INTERVAL)
	{
		*ProjectileSpawnIntervalTimer -= HEAL_PROJECTILE_SPAWN_INTERVAL;
		
		for (AC_BasicEnemy* HealTarget : Nurse->GetHealProjectileTargets())
		{
			// 발사 Direction 설정
			const FVector ToTargetDir    = (HealTarget->GetActorLocation() - Nurse->GetActorLocation()).GetSafeNormal();
			const FVector LaunchDirection = (FVector::UnitZ() + ToTargetDir).GetSafeNormal();
			
			ZOMBIE_MANAGER(this)->SpawnHealingProjectile
			(
				Nurse->GetActorLocation() + FVector::UnitZ() * 100.f,
				LaunchDirection,
				Nurse,
				HealTarget,
				30.f
			);
		}
	}
}
