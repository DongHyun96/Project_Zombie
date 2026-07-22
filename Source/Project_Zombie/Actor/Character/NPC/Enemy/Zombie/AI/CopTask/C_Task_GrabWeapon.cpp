// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_GrabWeapon.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/C_AIGunUsageComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/C_Util.h"

UC_Task_GrabWeapon::UC_Task_GrabWeapon()
{
}

EBTNodeResult::Type UC_Task_GrabWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AC_CopZombie* CopZombie = Cast<AC_CopZombie>(OwnerComp.GetAIOwner()->GetPawn());
	if (CopZombie->GetEquippedGun()) return EBTNodeResult::Failed; // 이미 무기를 장착 중인 상황

	// 무기뺏기 시도 실행
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UC_Task_GrabWeapon::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
	Super::TickTask(_OwnCom, _NodeMemory, _DeltaSeconds);
}

void UC_Task_GrabWeapon::OnTaskFinished
(
	UBehaviorTreeComponent& OwnerComp,
	uint8*					NodeMemory,
	EBTNodeResult::Type		TaskResult
)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// TaskResult가 Succeeded가 아닌 경우(Abort 처리되었거나 기타 등등), 최종적으로 무기를 빼앗는 처리를 하지 않는다
	if (TaskResult != EBTNodeResult::Succeeded) return;

	AC_CopZombie* CopZombie = Cast<AC_CopZombie>(OwnerComp.GetAIOwner()->GetPawn()); 
	if (!CopZombie)
	{
		UC_Util::Print("From UC_Task_GrabWeapon::OnTaskFinished : This Task is for CopZombie", FColor::Red, 10.f);
		return;
	}
	
	// 성공적으로 Skill이 끝났다고 판단, GrabRangeCollider에 들어온 Player들을 조사해서 제일 적합한 Player의 무기를 뺏는다
	float BestDistSqr = FLT_MAX;
	AC_BasicPlayer* BestGrabPlayer{};
	for (AC_BasicPlayer* GrabZoneEnteredPlayer : CopZombie->GetGrabRangeEnteredPlayers())
	{
		// 그로기 상태거나 이미 사망한 Player의 경우 대상에서 제외함
		if (GrabZoneEnteredPlayer->GetStatComponent()->IsCurHPZero()) continue;
		
		// 이미 MainWeaponSlot에 장착된 MainWeapon이 없는 Player 역시 대상에서 제외함
		if (!GrabZoneEnteredPlayer->GetEquippedComponent()->GetSlotWeapon(EWeaponSlot::MainWeapon)) continue;

		// GrabRangeCollider에 들어온 Player 중 가장 가까운 거리의 Player MainWeapon을 뺴앗는다
		const float CurDistSqr = FVector::DistSquared(GrabZoneEnteredPlayer->GetActorLocation(), CopZombie->GetActorLocation());
		
		if (CurDistSqr < BestDistSqr)
		{
			BestDistSqr    = CurDistSqr;
			BestGrabPlayer = GrabZoneEnteredPlayer;
		}
	}

	if (!BestGrabPlayer) return; // MainWeapon을 빼앗을 Player가 없음
	
	// MainWeapon을 뺏을 Player 대상이 있음 -> 대상의 Weapon을 CopZombie에게 부착 처리한다
	// Player의 MainWeaponSlot 없앰과 동시에, 탈취(PrevSlotWeapon return됨)
	AC_WeaponBase* StolenWeapon = BestGrabPlayer->GetEquippedComponent()->SetSlotWeapon(EWeaponSlot::MainWeapon, nullptr);
	AC_GunBase* StolenGun = Cast<AC_GunBase>(StolenWeapon);
	if (!StolenGun) return; // BestGrabPlayer의 이전 Weapon이 없었던 상태(애초에 위에서 체킹해서 이 방어코드로 들어오면 안되긴 함)
	
	// 뺏은 무기 장착 시도
	if (!CopZombie->EquipWeapon(StolenGun)) return;
	
	// 장착 성공, AIUsage에 이전 Player 주인 세팅
	StolenGun->GetAIGunUsageComponent()->SetPrevOwnerPlayer(BestGrabPlayer);
	
	
	// 제대로 장착 처리되었다면 MainState 키값 수정 (다른 Zombie는 Service에서 바꾸지만, 이 해당 키는 바로 바꿔주어야 해당 Task를 바로 실행)
	OwnerComp.GetBlackboardComponent()->SetValueAsEnum(m_MainState.SelectedKeyName, static_cast<uint8>(ECopZombieState::WeaponEarned));
}
