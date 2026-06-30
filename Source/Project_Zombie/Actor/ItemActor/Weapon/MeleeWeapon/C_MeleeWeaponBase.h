// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "C_MeleeWeaponBase.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_MeleeWeaponBase : public AC_WeaponBase
{
	GENERATED_BODY()

public:
	
	AC_MeleeWeaponBase();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

public:
	
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;
	
public:
	
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;

protected:

	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HandSocketName"))
	FName m_HandSocketName{};
	
	// Holster Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HolsterSocketName"))
	FName m_HolsterSocketName{};
	
};
