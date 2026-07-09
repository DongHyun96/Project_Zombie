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
	/// 스킬 구현함수
	/// </summary>
	virtual void Activate(class AC_BasicEnemy* _Owner, class UC_EnemySkillData* _Data);

	/// <summary>
	/// 애니메이션 Notify에서 호출되는 함수, 스킬의 실제 효과를 발동시키는 함수
	/// </summary>
	/// <param name="_Owner"></param>
	/// <param name="_Data"></param>
	virtual void Fire(class AC_BasicEnemy* _Owner, class UC_EnemySkillData* _Data);

public:
	UC_EnemySkillBase();
};
