// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTowerEditorSubsystem.h"

#include "EngineUtils.h"
#include "Actor/PointTower/C_PointTower.h"

void UC_PointTowerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FEditorDelegates::OnMapOpened.AddUObject(this, &UC_PointTowerEditorSubsystem::OnMapOpened);
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

	// 복사 또는 복제에 의해 생성된 PointTower의 경우, 따로 세팅을 해줄 필요가 없음
	if (!PointTower->m_PointTowerSettings.IsEmpty()) return;
	
	/* 새로이 추가된 PointTower에 대한 처리인 경우 */
	
	// 자기자신의 세팅값을, 이미 배치된 대응되는 ActiveIdx의 PointTower가 존재한다면 해당 PointTower로 세팅값 맞춤
	PointTower->TrySyncSelfAll();
}

void UC_PointTowerEditorSubsystem::OnMapOpened(const FString& _FileName, bool _bAsTemplate)
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

	if (!IsValid(EditorWorld)) return;

	for (TActorIterator<AC_PointTower> It(EditorWorld); It; ++It)
	{
		// PointTowerSettings 개수가 TimeOfDay 개수와 일치하지 않다면 해당 값 Default로 집어넣기 처리
		It->AddDefaultTowerSettingsIfNecessary();
	}
}
