// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Interact/C_InteractableBase.h"
#include "C_SkinStation.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_SkinStation : public AC_InteractableBase
{
	GENERATED_BODY()

public:
	AC_SkinStation();

protected:
	virtual void BeginPlay() override;
};
