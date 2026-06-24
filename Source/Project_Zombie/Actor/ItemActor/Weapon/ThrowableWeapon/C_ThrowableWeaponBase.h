// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "C_ThrowableWeaponBase.generated.h"

UENUM(BlueprintType)
enum class EThrowableWeaponType : uint8
{
	Grenade,
	FlashBang,
	Molotov		// 화염병
};

UCLASS()
class PROJECT_ZOMBIE_API AC_ThrowableWeaponBase : public AC_WeaponBase
{
	GENERATED_BODY()

public:
	AC_ThrowableWeaponBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;

protected:

	// ThrowableType 종류 (Blueprint에서 초기화해줄 것)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "ThrowableWeaponType"))
	EThrowableWeaponType m_ThrowableWeaponType{};

private: /* Socket Name 관련 */

	// Hand Socket Names
	static const TMap<EThrowableWeaponType, FName> s_HandSocketNames;
	
	// Holster(무기집 위치) Socket Name (모든 Throwable 공통 무기집 위치 사용할 예정)
	static const FName s_HolsterSocketName;
	
protected: // 충돌체 관련

	// Mesh의 Collision을 사용하지 않고, Capsule 모양 Collider를 사용하여 충돌 검사 및 처리를 진행할 예정
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "MainCollider"))
	UShapeComponent* m_MainCollider{};

	// TODO 폭발처리 반경 Collider 필요 -> 추후 추가할 것
	
protected:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "ProjectileMovementCom"))
	class UProjectileMovementComponent* m_ProjectileMovement{};
	
	
	
};
