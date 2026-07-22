// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "C_ThrowableWeaponBase.generated.h"

class AC_FireDamageArea;
class AC_BasicPlayer;

class USplineComponent;
class USplineMeshComponent;
class UStaticMeshComponent;

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
	Idle,		// 기본 상태
	RemovePin,	// 핀 제거
	Ready,		// 투척 준비 
	ReadyLoop,	// 투척 준비 동작 루프
	Throwing,	// 투척 중
	Thrown,		// 투척 
	Exploded,	// 폭발 
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

public:  // Getter & Setter
	
	float GetExplosionRadius() const { return m_ExplosionRadius; }
	void SetExplosionRadius(float _ExplosionRadius) { m_ExplosionRadius = _ExplosionRadius; }

	float GetMaxDamage() const { return m_MaxDamage; }
	void SetMaxDamage(float _MaxDamage) { m_MaxDamage = _MaxDamage; }

	float GetMinDamage() const { return m_MinDamage; }
	void SetMinDamage(float _MinDamage) { m_MinDamage = _MinDamage; }

	ECollisionChannel GetExplosionTraceChannel() const { return m_ExplosionTraceChannel; }

	TSubclassOf<AC_FireDamageArea> GetFireDamageAreaClass() const { return m_FireDamageAreaClass; }

	const FHitResult& GetHitResult() const { return m_HitResult; }

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
	void CancelThrowAction();

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

	/// <summary>
	///  충돌 이벤트 함수
	/// </summary>
	/// <param name="HitComponent">충돌한 컴포넌트</param>
	/// <param name="OtherActor">충돌한 액터</param>
	/// <param name="OtherComp">충돌한 액터의 컴포넌트</param>
	/// <param name="NormalImpulse">충돌 시 발생한 힘의 방향과 크기</param>
	/// <param name="Hit">충돌 정보</param>
	UFUNCTION()
	void OnThrowableHit
	(
		UPrimitiveComponent* HitComponent,	
		AActor* OtherActor,					
		UPrimitiveComponent* OtherComp,		
		FVector NormalImpulse,				
		const FHitResult& Hit				
	);


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


private: // 예상 경로 표시 관련

	/// <summary>
	/// 현재 조준 방향을 기준으로 예상 경로 계산하고 표시
	/// </summary>
	void UpdatePredictedPath();


	/// <summary>
	/// 표시된 예상 경로와 충돌 위치 표시 제거
	/// </summary>
	void ClearPredictedPath();


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

	// 몽타주
	//UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	//UAnimMontage* m_RemovePinMontage;

	//UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	//UAnimMontage* m_ReadyMontage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* m_ThrowMontage;

public: 

	// Section 이름들
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName m_RemovePinSectionName;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName m_ReadySectionName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName m_LoopSectionName;
	
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

	// 충돌 시 폭발 여부
	// 충돌하면 바로 폭발하는가?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|State")
	bool m_bExplodeOnImpact;

	// 폭발까지 걸리는 시간 (핀 제거 후, 폭발까지 걸리는 시간)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|State",
		meta = (EditCondition = "!m_bExplodeOnImpact", EditConditionHides, ClampMin = "0.0"))
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

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Launch")
	TSubclassOf<class UObject> m_ExplodeStrategyClass;

	// 장판 데미지 영역 클래스
	// 일단 화염병 전용으로 AC_FireDamageArea를 사용하지만, 나중에 다른 장판 데미지 영역이 생기면 수정 예정	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Fire Damage Area")
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Damage Area")
	TSubclassOf<AC_FireDamageArea> m_FireDamageAreaClass;

	// 폭발 반경
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Explosion")
	float m_ExplosionRadius;

	// 최대 데미지
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Explosion")
	float m_MaxDamage;

	// 최소 데미지
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Explosion")
	float m_MinDamage;

	// 폭발 데미지를 줄 때 사용할 Trace Channel
	// 폭발 위치와 대상 사이에 벽 있는지 확인
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Explosion")
	TEnumAsByte<ECollisionChannel> m_ExplosionTraceChannel = ECC_Visibility;

	/// ---------나중에 스킬 데이터로 따로 빠질수도-----------
	// 폭발 이펙트 
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Effect")
	TObjectPtr<UParticleSystem> m_ExplosionEffect;

	// 폭발 이펙트 크기 (1.0 = 기본 크기)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Effect")
	float m_ExplosionEffectScale;


protected:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "ProjectileMovementCom"))
	class UProjectileMovementComponent* m_ProjectileMovement{};


protected:

	// Throwable Weapon의 타입
	EThrowableType m_ThrowableType;

	// Throwable Weapon의 상태
	EThrowableState m_ThrowableState;

	// 폭발 실제 Object
	UPROPERTY()
	UObject* m_ExplodeStrategyObject = nullptr;

	// 충돌 시 충돌 정보 저장
	// 벽에 충돌했을때 장판이 생성되는 위치를 결정하기 위해 사용
	FHitResult m_HitResult;

protected: // 예상 투척 경로

	// 예상 경로의 위치들을 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable|Predicted Path")
	USplineComponent* m_PathSpline;

	// 예상 경로가 끝나는 위치를 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable|Predicted Path")
	UStaticMeshComponent* m_PredictedEndPoint;

	// 예상 경로를 표시할 Mesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Predicted Path")
	UStaticMesh* m_PredictedPathMesh;

	// 예상 경로를 몇 초 동안 계산할지 결정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Predicted Path")
	float m_PredictedPatchMaxTime;

	// 예상 경로를 1 초 동안 몇번 계산할지 결정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Predicted Path")
	float m_PredictedPatchSimFrequency;

	// 예상 경로를 계산할 때 사용할 Trace Channel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Predicted Path")
	TEnumAsByte<ECollisionChannel> m_PredictedPatchTraceChannel;


	// 예상 경로를 표시할 Mesh들을 배열에 저장
	UPROPERTY(Transient)
	TArray<USplineMeshComponent*> m_PredictedPathMeshes;

private:

	// 투척류의 Fuse Timer (핀 제거 후, 폭발까지 걸리는 시간)
	FTimerHandle m_FuseTimerHandle;
	
	// 버튼을 누르고 있는지
	bool m_bIsCharging;

	// 마우스를 떼었는지
	bool m_bWantsThrow;

	// 쿠킹을 원하는지
	bool m_bWantsCook;
};
