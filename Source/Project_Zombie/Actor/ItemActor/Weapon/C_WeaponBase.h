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
	// Sets default values for this actor's properties
	AC_WeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	/// <summary>
	/// 무기집에 무기 붙이기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHolster, return false;);

	/// <summary>
	/// 손에 장착하기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHand(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHand, return false;);

};
