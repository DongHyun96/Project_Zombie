// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "C_ZombieController.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_ZombieController : public AAIController
{
	GENERATED_BODY()
	
protected:
	// 인지기능
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAIPerceptionComponent*	m_PerceptionCom; 

	// 시야, 시각정보
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAISenseConfig_Sight*		m_SightConfig;

	// 데미지 정보
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAISenseConfig_Damage*	m_DamageConfig;

	// 청각 정보
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAISenseConfig_Hearing*	m_HearingConfig;

	// 비헤이비어트리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree*			m_BTAsset;

	// 블랙보드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBlackboardData*			m_BBAsset;

public:
	/// <summary>
	/// 타겟 감지함수
	/// </summary>
	/// <param name="_Target"></param>
	/// <param name="_Stimulus"></param>
	UFUNCTION()
	void OnTargetDetected(AActor* _Target, FAIStimulus _Stimulus);

protected:
	/// <summary>
	/// AIController 가 Zombie 를 소유하면 호출되는 함수
	/// </summary>
	/// <param name="_Pawn"></param>
	virtual void OnPossess(APawn* _Pawn) override;

public:
	AC_ZombieController();

};
