// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Anim/C_ZombieAnimInstance.h"
#include "C_CopZombieAnimInstance.generated.h"

enum class ECopZombieState : uint8;

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_CopZombieAnimInstance : public UC_ZombieAnimInstance
{
	GENERATED_BODY()

public:
	
	UC_CopZombieAnimInstance();
	
public:
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float _DT) override;

private:
	
	UPROPERTY()
	class AC_CopZombie* m_OwnerCopZombie{};
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CopZombie")
	ECopZombieState m_CopZombieState{};

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector m_LeftHandIKTranslation{};
	
};
