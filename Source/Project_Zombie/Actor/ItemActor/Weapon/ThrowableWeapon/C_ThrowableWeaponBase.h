// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "C_ThrowableWeaponBase.generated.h"

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


public:
	
	virtual bool OnStartFire(class AC_BasicPlayer* _WeaponUser) override;
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) override;
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) override;
	
public: // 애님 노티파이 관련

	// Blueprint에서 사용 가능하도록 UFUNCTION으로 선언
	/// <summary>
	/// 핀 제거 동작 애님 노티파이 이벤트
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Throwable|AnimNotify")
	void OnRemovePin();

	/// <summary>
	/// 차징 준비 동작 애님 노티파이 이벤트
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Throwable|AnimNotify")
	void OnThrowReadyLoop();

	/// <summary>
	/// 투척 동작 애님 노티파이 이벤트
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Throwable|AnimNotify")
	void OnThrowThrowable();


	/* Socket Name 관련 */
protected: 

	// Hand Socket Name (각 Throwable 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HandSocketName"))
	FName m_HandSocketName{};
	
private:
	
	// Holster(무기집 위치) Socket Name (모든 Throwable 공통 무기집 위치 사용할 예정)
	static const FName s_HolsterSocketName;
	
protected: // 충돌체 관련

	// Mesh의 Collision을 사용하지 않고, Capsule 모양 Collider를 사용하여 충돌 검사 및 처리를 진행할 예정
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "MainCollider"))
	UShapeComponent* m_MainCollider{};

	// TODO 폭발처리 반경 Collider 필요 -> 추후 추가할 것
	

public: // 몽타주 관련
	
	// 핀 제거 동작 몽타주
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* m_RemovePinMontage;

	// 투척 준비 동작 몽타주
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* m_ReadyMontage;

	// 투척 동작 몽타주
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* m_ThrowMontage;


protected:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "ProjectileMovementCom"))
	class UProjectileMovementComponent* m_ProjectileMovement{};


protected:

	// 이 Throwable Weapon을 사용하는 Player
	UPROPERTY()
	AC_BasicPlayer* m_WeaponUser;

	// 투척 과정 중인지
	UPROPERTY(BlueprintReadOnly, Category = "Throwable State")
	bool m_bIsThrowing;
	
	// 버튼을 누르고 있는지
	UPROPERTY(BlueprintReadOnly, Category = "Throwable State")
	bool m_bIsCharging;

	// 쿠킹이 시작되었는지
	UPROPERTY(BlueprintReadOnly, Category = "Throwable State")
	bool m_bIsCooking;

protected: 

	// 투척류의 Fuse Time (핀 제거 후, 폭발까지 걸리는 시간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable Weapon")
	float m_FuseTime;

	// 남은 Fuse Time (핀 제거 후, 폭발까지 남은 시간)
	UPROPERTY(BlueprintReadOnly, Category = "Throwable Weapon")
	float m_RemainingFuseTime;
};
