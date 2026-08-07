// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_RotateToTarget.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UC_Task_RotateToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AC_ZombieController* pController = Cast<AC_ZombieController>(OwnerComp.GetAIOwner());
	if (!pController) return EBTNodeResult::Failed;

	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());
	if (!pZombie) return EBTNodeResult::Failed;

	UBlackboardComponent* pBBCom = OwnerComp.GetBlackboardComponent();
	if (!pBBCom) return EBTNodeResult::Failed;
	
	AActor* TargetActor = Cast<AActor>(pBBCom->GetValueAsObject(m_Target.SelectedKeyName));
	if (!TargetActor) return EBTNodeResult::Failed;
	
	// 1. Zombie -> TargetActor 방향 벡터 구하기
	const FVector ZombieLocation = pZombie->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
    
	FVector Direction = TargetLocation - ZombieLocation;
	Direction.Z = 0.f; // 캐릭터가 위/아래로 기울어지지 않도록 Z축 고정

	if (Direction.IsNearlyZero()) return EBTNodeResult::Succeeded;

	// 2. 방향 벡터로부터 회전값 계산
	FRotator TargetRotation = Direction.Rotation();

	// 3. +- 30도 범위의 랜덤 오차 추가 및 Normalize
	float RandomYawOffset = FMath::FRandRange(-30.f, 30.f);
	TargetRotation.Yaw   = FRotator::NormalizeAxis(TargetRotation.Yaw + RandomYawOffset);
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll  = 0.f;

	// 4. 즉시 회전 적용
	pZombie->SetActorRotation(TargetRotation);
    
	// AI Controller의 ControlRotation도 함께 갱신 (Pawn의 bUseControllerRotationYaw 옵션 대응)
	pController->SetControlRotation(TargetRotation);

	return EBTNodeResult::Succeeded;
}
