// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C_BasicEnemy.h"
#include "C_Zombie.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_Zombie : public AC_BasicEnemy
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "StatComponent"))
	class UC_ZombieStatComponent* m_StatCom;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	AC_Zombie();
};
