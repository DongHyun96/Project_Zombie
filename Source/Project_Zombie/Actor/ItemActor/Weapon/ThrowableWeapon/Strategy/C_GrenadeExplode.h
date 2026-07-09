// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../Interface/I_ExplodeStrategy.h"
#include "C_GrenadeExplode.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_GrenadeExplode : public UObject, public II_ExplodeStrategy
{
	GENERATED_BODY()
	
public:
	/// <summary>
	/// 인터페이스 구현, 수류탄 폭발 처리
	/// </summary>
	/// <param name="_TrhowableWeapon">투척류</param>
	/// <returns>폭발 처리 성공 여부</returns>
	virtual bool UseStrategy_Implementation(AC_ThrowableWeaponBase* _ThrowableWeapon) override;
};
