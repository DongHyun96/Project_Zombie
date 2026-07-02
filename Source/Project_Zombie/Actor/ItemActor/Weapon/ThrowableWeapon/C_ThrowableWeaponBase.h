// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "C_ThrowableWeaponBase.generated.h"

UENUM(BlueprintType)
enum class EThrowableType : uint8
{
	None,
	Grenade,
	Molotov,
};

UENUM(BlueprintType)
enum class EThrowableState : uint8
{
	None,
	RemovePin,
	Ready,
	Thorw,
	Thrown,
	Exploded,
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

private:
	/// <summary>
	/// 투척 취소 동작
	/// </summary> 
	void CancleThrowAction();

	/// <summary>
	///	Throwable Weapon의 상태 초기화
	/// </summary>
	void ResetThrowableState();


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

	// 몽타주 (몽타주 안에서 세션 나누어서 애님 노티파이 이벤트 발생)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* m_ThrowMontage;

	// Section 이름들
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName m_RemovePinSectionName;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName m_ReadySectionName;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName m_ThrowSectionName;

public: // Throwable Weapon의 특성 관련
	
	// 핀 제거 가능 여부
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool m_bHasPin;

	// 쿠킹 가능 여부 (R키를 눌렀을 때)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool m_bIsCookable;

	// 폭발까지 걸리는 시간 (핀 제거 후, 폭발까지 걸리는 시간)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float m_FuseTime;

	// 투척 속도
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float m_ThrowSpeed;

protected:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "ProjectileMovementCom"))
	class UProjectileMovementComponent* m_ProjectileMovement{};


protected:

	// 이 Throwable Weapon을 사용하는 Player
	UPROPERTY()
	AC_BasicPlayer* m_WeaponUser;

	// Throwable Weapon의 타입
	EThrowableType m_ThrowableType;

	// Throwable Weapon의 상태
	EThrowableState m_ThrowableState;


private:
	// 투척 과정 중인지
	bool m_bIsThrowing;
	
	// 버튼을 누르고 있는지
	bool m_bIsCharging;

	// 쿠킹이 시작되었는지
	bool m_bIsCooking;

	// 마우스를 떼었는지
	bool m_bWantsThrow;

protected: 

	//// 투척류의 Fuse Time (핀 제거 후, 폭발까지 걸리는 시간)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable Weapon")
	//float m_FuseTime;

	//// 남은 Fuse Time (핀 제거 후, 폭발까지 남은 시간)
	//UPROPERTY(BlueprintReadOnly, Category = "Throwable Weapon")
	//float m_RemainingFuseTime;
};
