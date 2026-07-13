// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_Zombie.h"
#include "C_NurseZombie.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_NurseZombie : public AC_Zombie
{
	GENERATED_BODY()

public:
	
	AC_NurseZombie();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;
};
