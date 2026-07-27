// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ChargeAttack.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

#include "Actor/Character/NPC/Enemy/Zombie/TankZombie/C_TankZombie.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Utility/C_Util.h"

UC_ChargeAttack::UC_ChargeAttack()
{
}

bool UC_ChargeAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	if (!IsValid(_Owner) || !IsValid(_Data))
		return false;

	AAIController* Controller = Cast<AAIController>(_Owner->GetController());
	if (!IsValid(Controller))
		return false;

	UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent();
	if (!IsValid(Blackboard))
		return false;

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("Target")));
	if (!IsValid(Target))
		return false;

	AC_TankZombie* Tank = Cast<AC_TankZombie>(_Owner);
	if (!IsValid(Tank))
		return false;

	if (!IsValid(_Data->Montage))
		return false;

	USkeletalMeshComponent* Mesh = Tank->GetMesh();
	if (!IsValid(Mesh))
		return false;

	UAnimInstance* AnimInst = Mesh->GetAnimInstance();
	if (!IsValid(AnimInst))
		return false;

	// 이미 돌진중이면 다시 실행하지않음
	if (Tank->IsCharging())
		return false;

	// Notify가 실행될 때 사용할 타겟과 스킬데이터 저장
	if (!Tank->PrepareCharge(Target, _Data))
		return false;

	// 몽타주 재생
	const float MontageLength = AnimInst->Montage_Play(_Data->Montage);

	if (MontageLength <= 0.f)
	{
		Tank->CancelPrepareCharge();
		return false;
	}


	return true;
}
