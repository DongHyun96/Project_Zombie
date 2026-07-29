// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "C_EnemySkillBase.generated.h"

// 모든 스킬의 최상위 클래스, 스킬의 공통적인 기능을 정의
UCLASS()
class PROJECT_ZOMBIE_API UC_EnemySkillBase : public UObject
{
	GENERATED_BODY()
	
public:
	/// <summary>
	/// 스킬 Activate 시작 처리 함수 (Server 환경에서만 실행 처리되는 함수)
	/// </summary>
	/// <param name="_Owner"></param>
	/// <param name="_Data"></param>
	/// <param name="_PlayedMontageSectionIdx"> : (OUT Ref) 스킬 실행에 성공하였다면, 재생된 MontageSection 인덱스 초기화시켜서 반환 처라 </param>
	/// <returns> : 스킬을 실행할 수 없는 상황이라면 return false </returns>
	virtual bool Activate(class AC_BasicEnemy* _Owner, class UC_EnemySkillData* _Data, OUT int32& _PlayedMontageSectionIdx);

	/// <summary>
	/// 애니메이션 Notify에서 호출되는 함수, 스킬의 실제 효과를 발동시키는 함수
	/// </summary>
	/// <param name="_Owner"></param>
	/// <param name="_Data"></param>
	virtual void Fire(class AC_BasicEnemy* _Owner, class UC_EnemySkillData* _Data);

public:
	UC_EnemySkillBase();
};
