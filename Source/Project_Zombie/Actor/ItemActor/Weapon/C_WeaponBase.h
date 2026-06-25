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
	
protected:

	// 해당 무기의 무기 꺼내는 동작 Montage
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "DrawMontage"))
	UAnimMontage* m_DrawMontage{};
	
	// 해당 무기의 무기집에 집어넣는 동작 Montage
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "SheathMontage"))
	UAnimMontage* m_SheathMontage{};
	
	


};
