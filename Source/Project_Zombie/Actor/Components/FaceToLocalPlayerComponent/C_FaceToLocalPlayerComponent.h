// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "C_FaceToLocalPlayerComponent.generated.h"

/**
 * 특정 Component들(WidgetCom 등) Local Player(Local 환경 플레이어) 바라보기 처리
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_FaceToLocalPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UC_FaceToLocalPlayerComponent();
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	/// <summary>
	/// Local Player 방면으로 지속적으로 회전시킬 Component 등록
	/// </summary>
	/// <param name="_SceneComponent"></param>
	UFUNCTION(BlueprintCallable)
	void RegisterSceneComToRotate(USceneComponent* _SceneComponent);

	void ToggleEnableRotation(bool _Enable) { m_EnableRotation = _Enable; }
	
private:
	
	UPROPERTY()
	TArray<USceneComponent*> m_RotatingSceneComponents{};

protected:
	
	UPROPERTY(BlueprintReadWrite)
	bool m_EnableRotation = true;
	
};
