// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_UtilActor.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_UtilActor : public AActor
{
	GENERATED_BODY()

public:
	
	AC_UtilActor();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

public:
	
	const FColor& GetCurTickColor() const { return m_CurTickColor; }
	
private:
	
	FColor m_CurTickColor = FColor::Red;
	
};
