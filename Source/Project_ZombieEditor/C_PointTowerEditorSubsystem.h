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

	/// <summary>
	/// <para> EditInstanceOnly로 세팅된 PointTowerSettings까지 모두 불려와진 상황 -> 이 때 모종의 이유로 디폴트로도 세팅된 값이 없는 PointTower에 대해 </para>
	/// <para> 자동적으로 Default setting 값을 넣어주기 위함 </para>
	/// <para> TimeOfDay 개수와 일치해야 한다 </para>
	/// </summary>
	void OnMapOpened(const FString& _FileName, bool _bAsTemplate);
	
};
