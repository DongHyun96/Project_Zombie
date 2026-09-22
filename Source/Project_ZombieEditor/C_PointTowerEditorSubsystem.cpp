// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTowerEditorSubsystem.h"

#include "Actor/PointTower/C_PointTower.h"

void UC_PointTowerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	GEngine->OnLevelActorAdded().AddUObject(this, &UC_PointTowerEditorSubsystem::OnLevelActorAdded);
}

void UC_PointTowerEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	GEngine->OnLevelActorAdded().RemoveAll(this);
}

void UC_PointTowerEditorSubsystem::OnLevelActorAdded(AActor* _Actor)
{
	AC_PointTower* PointTower = Cast<AC_PointTower>(_Actor);
	if (!PointTower) return;

	UWorld* World = PointTower->GetWorld();
	
	if (!World || World->WorldType != EWorldType::Editor) return;

	UE_LOG(LogTemp, Warning, TEXT("UC_PointTowerEditorSubsystem::OnLevelActorAdded %s"), *(_Actor->GetName()));
	
	// 새로이 추가된 PointTower에 대한 처리
	// 자기자신의 세팅값을, 이미 배치된 대응되는 ActiveIdx의 PointTower가 존재한다면 해당 PointTower로 세팅값 맞춤 
	PointTower->TrySyncSelf();
}
