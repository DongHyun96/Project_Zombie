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
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) override;
	
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

public: // 쿠킹 입력

	/// <summary>
	/// R키를 눌렀을 때 쿠킹 시작 처리
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Throwable|Cooking")
	bool OnStartCookInput();

protected: // 폭발

	/// <summary>
	/// 폭발 처리
	/// 실제 폭방은 I_ExplodeStrategy를 상속받은 클래스에서 처리할 예정
	/// </summary>
	void Explode();

private:
	/// <summary>
	/// 투척 취소 동작
	/// </summary> 
	void CancleThrowAction();

	/// <summary>
	///	Throwable Weapon의 상태 초기화
	/// </summary>
	void ResetThrowableState();

private: // 투척 관련 처리

	/// <summary>
	/// 플레이어가 바라보는 방향을 기준으로 투척 방향 반환
	/// </summary> 
	FVector GetThrowDirection() const;

	/// <summary>
	/// 손에 들고 있는 위치를 기준으로 투척 시작 위치 반환
	/// </summary> 
	FVector GetLaunchLocation(const FVector& _ThrowDirection) const;

	/// <summary>
	/// 충돌 활성화하고 플레이어와 충돌하지 않도록 설정
	/// </summary> 
	void SetupThrowCollision();

	/// <summary>
	/// 투척 시작 위치와 방향을 기준으로 Projectile Movement Component를 사용하여 투척
	///	</summary>
	void LaunchCurrentActorAsProjectile(const FVector& _ThrowDirection);

private: // 타이머 관련

	/// <summary>
	///  타이머를 가지고 있는지 여부 반환
	/// </summary>
	bool HasFuseTimer() const;

	/// <summary>
	/// 폭발 타이머 시작 
	/// R키 쿠킹 or 던지는 순간에 호출
	/// </summary>
	bool StartFuseTimer();

	/// <summary>
	/// 폭발 타이머 제거
	/// </summary>
	void ClearFuseTimer();

	/// <summary>
	/// 폭발 타이머가 끝났을 때 호출
	/// </summary>
	void OnFuseTimerFinished();

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

public: // Throwable Weapon의 투척 특성 관련
	
	// 핀 제거 가능 여부 
	// 핀 제거 동작 몽타주를 넣을 것인가?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|State")
	bool m_bHasPin;

	// 쿠킹 가능 여부 
	// R키를 눌렀을 때 쿠킹 가능한가?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|State")
	bool m_bIsCookable;

	// 폭발까지 걸리는 시간 (핀 제거 후, 폭발까지 걸리는 시간)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|State")
	float m_FuseTime;

	// 투척 속도
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|State")
	float m_ThrowSpeed;

	// Player의 Hand Socket 위치에서 Forward 방향으로 Offset만큼 이동한 위치에서 투척
	// Player Collider와 충돌하는 문제를 방지하기 위해
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Launch")
	float m_LaunchForwardOffset;

	// Player의 Hand Socket 위치에서 Upward 방향으로 Offset만큼 이동한 위치에서 투척
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Launch")
	float m_LaunchUpwardOffset;

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

	// 투척류의 Fuse Timer (핀 제거 후, 폭발까지 걸리는 시간)
	FTimerHandle m_FuseTimerHandle;

	// 투척 과정 중인지
	bool m_bIsThrowing;
	
	// 버튼을 누르고 있는지
	bool m_bIsCharging;

	// 쿠킹이 시작되었는지
	bool m_bIsCooking;

	// 마우스를 떼었는지
	bool m_bWantsThrow;

	// 쿠킹을 원하는지
	bool m_bWantsCook;

	// 폭발이 발생했는지
	bool m_bHasExploded;
};
