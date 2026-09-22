// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "C_PointTowerEditorSubsystem.generated.h"

/**
 * Level에 새로이 PointTower를 배치할 때, 세팅값 싱크를 맞추기 위함
 */
UCLASS()
class PROJECT_ZOMBIEEDITOR_API UC_PointTowerEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	
	void OnLevelActorAdded(AActor* _Actor);
	
};
