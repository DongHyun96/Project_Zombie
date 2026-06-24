// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_WeaponBase.generated.h"

UCLASS(Abstract)
class PROJECT_ZOMBIE_API AC_WeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_WeaponBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	virtual void StartAttack() PURE_VIRTUAL(AC_WeaponBase::StartAttack, );
	virtual void StopAttack() PURE_VIRTUAL(AC_WeaponBase::StopAttack, );
	virtual void Reload() PURE_VIRTUAL(AC_WeaponBase::Reload, );

};
