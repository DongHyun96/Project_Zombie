// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_ExplodeStrategy.generated.h"

class AC_ThrowableWeaponBase;

// 전략 인터페이스
UINTERFACE(MinimalAPI)
class UI_ExplodeStrategy : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_ZOMBIE_API II_ExplodeStrategy
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 투척류는 이 인터페이스를 구현해서 각자 다른 실제 폭발 방식을 처리
	/// </summary>
	/// 기본은 C++에서 구현, BlueprintOverride 가능
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) 
	bool UseStrategy(AC_ThrowableWeaponBase* _ThrowableWeapon);
	virtual bool UseStrategy_Implementation(AC_ThrowableWeaponBase* _ThrowableWeapon) = 0;
};
