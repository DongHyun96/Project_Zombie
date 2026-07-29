// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ToxicAttack.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Utility/C_Util.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/Projectile/C_EnemyProjectile.h"

UC_ToxicAttack::UC_ToxicAttack()
{
}

bool UC_ToxicAttack::Activate(AC_BasicEnemy* _Owner, UC_EnemySkillData* _Data, OUT int32& _PlayedMontageSectionIdx)
{
	//UE_LOG(LogTemp, Warning, TEXT("PoisonAttack Activate"));

	_Owner->PlayAnimMontage(_Data->Montage);
	_PlayedMontageSectionIdx = 0;
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

	USkeletalMeshComponent* Mesh = _Owner->GetMesh();

	FVector SpawnLocation = Mesh->GetSocketLocation(TEXT("MouthSocket"));
	FRotator SpawnRotation = Mesh->GetSocketRotation(TEXT("MouthSocket"));

	// Fire Notify가 실행된 순간의 플레이어 위치
	FVector TargetLocation = Target->GetActorLocation();

	// 플레이어 아래 바닥 검색
	const FVector TraceStart = Target->GetActorLocation() + FVector::UpVector * 100.f;
	const FVector TraceEnd = Target->GetActorLocation() - FVector::UpVector * 1000.f;

	FHitResult GroundHit;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ToxicAttackTargetGround), false);

	QueryParams.AddIgnoredActor(_Owner);
	QueryParams.AddIgnoredActor(Target);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	const bool bFoundTargetGround = World->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, ObjectParams, QueryParams);

	if (bFoundTargetGround)
	{
		TargetLocation = GroundHit.ImpactPoint;
	}

	AC_EnemyProjectile* Projectile =
		GetWorld()->SpawnActor<AC_EnemyProjectile>(
			_Data->ProjectileClass,
			SpawnLocation,
			SpawnRotation);

	if (!Projectile)
	{
		UC_Util::Print("SpawnActor Failed");
		return;
	}
	else
	{
		Projectile->InitProjectile(_Owner, _Data, TargetLocation);
	}


	Super::Fire(_Owner, _Data);
}

