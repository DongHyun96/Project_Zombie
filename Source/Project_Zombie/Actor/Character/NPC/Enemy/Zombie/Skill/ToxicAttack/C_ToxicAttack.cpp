// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ToxicAttack.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/Projectile/C_EnemyProjectile.h"

UC_ToxicAttack::UC_ToxicAttack()
{
}

bool UC_ToxicAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	//UE_LOG(LogTemp, Warning, TEXT("PoisonAttack Activate"));

	_Owner->PlayAnimMontage(_Data->Montage);
	return true;
}

void UC_ToxicAttack::Fire(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data)
{
	//UE_LOG(LogTemp, Warning, TEXT("Fire"));

	if (!_Owner || !_Data)
		return;

	if (!_Data->ProjectileClass)
		return;

	AAIController* Controller = Cast<AAIController>(_Owner->GetController());
	if (!Controller)
		return;

	UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent();
	if (!Blackboard)
		return; 

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("Target")));
	if (!Target)
		return;

	UWorld* World = _Owner->GetWorld();
	if (!World)
		return;

	FVector SpawnLocation = _Owner->GetActorLocation();
	FRotator SpawnRotation = _Owner->GetActorRotation();

	// Fire Notify가 실행된 순간의 플레이어 위치
	const FVector TargetLocation = Target->GetActorLocation();

	AC_EnemyProjectile* Projectile =
		GetWorld()->SpawnActor<AC_EnemyProjectile>(
			_Data->ProjectileClass,
			SpawnLocation,
			SpawnRotation);

	if (!Projectile)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor Failed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActor Success"));
	}

	if (Projectile)
	{
		Projectile->InitProjectile(_Owner, _Data, TargetLocation);
	}


	Super::Fire(_Owner, _Data);
}

