// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Components/InteractionComponent/Strategy/C_InteractionStrategyBase.h"
#include "C_PlayerStatUpWidgetStrategy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerStatUpWidgetStrategy : public UC_InteractionStrategyBase
{
	GENERATED_BODY()
	
public:
	UC_PlayerStatUpWidgetStrategy();
	
	/// <summary>
	/// 하위 전략 대상과 상호작용할 수 있는지 검사
	/// </summary>
	virtual bool CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const;

	/// <summary>
	/// 화면에 표시할 상호작용 문구
	/// </summary>
	// virtual FText GetInteractionText(AC_BasicPlayer* _Interactor) const;


	/// ==============================================
	/// 상호작용 실행 
	/// ==============================================
	/// <summary>
	///	서버에서 상호작용을 시작할 때 호출	
	/// </summary>
	virtual bool StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor);

	/// <summary>
	/// 상호작용이 취소됐을 때 호출
	/// </summary>
	virtual void CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor);

	/// <summary>
	/// 상호작용이 완료됐을 때 호출
	/// </summary>
	virtual void CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor);
};
