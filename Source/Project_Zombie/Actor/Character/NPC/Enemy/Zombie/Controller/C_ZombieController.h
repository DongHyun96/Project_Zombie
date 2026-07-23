// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "C_ZombieController.generated.h"

// 감지된 타겟 정보 구조체
USTRUCT(BlueprintType)
struct FSensedTargetInfo
{
	GENERATED_BODY();
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<AActor>	Target;

	// 어그로 수치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float					AggroValue = 0.f;

	// 인지 유무
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool					bSensed;

	// 마지막으로 확인된 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector					LosePos;

	// 인지를 놓친 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float					LoseTime;
};

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

	// 감지된 타겟들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TArray<FSensedTargetInfo>		m_SensedTargets;


public:
	
	virtual void BeginPlay() override;
	
public:
	const TArray<FSensedTargetInfo>& GetSensedTargets() { return m_SensedTargets; }

	FSensedTargetInfo& AddSensedTarget(AActor* _Target);
	FSensedTargetInfo* FindSensedTarget(const AActor* _Target);
	void ClearSensedTarget(float _LimitTime);

	/// <summary>
	/// 현재 BB에 세팅된 Target Get
	/// </summary>
	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentBBTarget() const;
	
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
	
	/// <summary>
	/// 해당 Actor가 PerceptionComponent의 Sight에 실시간으로 잡혀있는 상황인지 체크
	/// </summary>
	/// <param name="_TargetActor"> 검사해 볼 Actor </param>
	/// <returns> : 시야에 들어와 있는 상황(SightConfig에 잡힌 보이는 상황) 이면 return true </returns>
	bool IsCurrentlyOnSight(AActor* _TargetActor) const;
	
public:
	AC_ZombieController();

};
