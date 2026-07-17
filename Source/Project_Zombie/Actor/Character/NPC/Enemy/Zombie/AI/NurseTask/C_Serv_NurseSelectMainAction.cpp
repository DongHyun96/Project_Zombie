// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Serv_NurseSelectMainAction.h"

UC_Serv_NurseSelectMainAction::UC_Serv_NurseSelectMainAction()
{
	Interval        = 3.f; // 간격
	RandomDeviation = 0.5f; // 랜덤편차
}

void UC_Serv_NurseSelectMainAction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	
}
