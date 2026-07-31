// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "C_NormalZombie.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_NormalZombie : public AC_Zombie
{
	GENERATED_BODY()

public:
	
	AC_NormalZombie();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* m_NormalAttackCollider{};
	
};
