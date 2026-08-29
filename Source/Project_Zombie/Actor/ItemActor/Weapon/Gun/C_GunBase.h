// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "GameFramework/Actor.h"
#include "../C_WeaponBase.h"
#include "C_GunBase.generated.h"

// 상황 : 이 모든 FireMode를 지원하지 않는 총기일 수 있음
// TArray<EFireMode> m_ApplicableFireMode -> 해당 배열에 총기 자신이 지원하는 FireMode를 넣고, EFireMode m_CurrentFireMode -> 이런식으로 둘 수 있을 듯
/// <summary>
/// 총기 조정간 FireMode Type
/// </summary>
UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single,		// 단발
	Burst,		// 점사
	FullAuto,	// 연발
	End			UMETA(Hidden) 
};

UCLASS()
class PROJECT_ZOMBIE_API AC_GunBase : public AC_WeaponBase
{
	GENERATED_BODY()

	friend class UC_AIGunUsageComponent;

protected:
	
	UPROPERTY()
	class AC_BasicEnemy* m_OwnerEnemy{};
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class USphereComponent* m_Collision{};

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Mesh", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* m_WeaponMesh{};		// 정적정보 - 상연, 데이터 테이블에서 가져와서 초기화 해주기.

	// AI Enemy가 Gun을 사용하는 처리 기능 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (DisplayName = "AIGunUsageComponent"))
	UC_AIGunUsageComponent* m_AIGunUsageComponent{};
	
protected:
	// 총이 갖는 최종 데미지
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	float					m_Damage{};

	// 현재 남아있는 총알 수
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Stats")
	int32					m_CurrentAmmo{};

	// 총이 갖는 최종 MaxAmmo
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	int32					m_MaxAmmo{};

	// 총이 갖는 최종 FireRate
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	float					m_FireRate{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GunStats")
	float m_SpreadAngle = 0.0f;

	UPROPERTY()
	class UAnimMontage* m_PlayerFireAnimation{};

	UPROPERTY()
	class UAnimMontage* m_PlayerReloadAnimation{};

	UPROPERTY(Transient)
	TObjectPtr<class UAnimSequence> m_FireAnimation{};   		// 정적정보. - 상연, 데이터 테이블에서 가져와서 초기화 해주기.
	
	UPROPERTY(Transient)
	TObjectPtr<class UAnimSequence> m_ReloadAnimation{};		// 정적정보. - 상연, 데이터 테이블에서 가져와서 초기화 해주기.
	
	UPROPERTY(Transient)
	TObjectPtr<class UStaticMesh> m_ShellMesh{};				// 정적정보. - 상연, 데이터 테이블에서 가져와서 초기화 해주기.

	// 이 무기의 원본 데이터 및 동적 데이터(CustomData)를 보유하는 통합 Entry
	// TODO : Lagacy로 지워질 예정
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	//FInventoryEntry			ItemEntry;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UNiagaraSystem* m_ShellEjectNiagaraSystem{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	class UParticleSystem* m_TracerFX{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	class UParticleSystem* m_ImpactFX{};

protected:
	// 현재 사격 버튼을 누르고 있는 상태인지 확인
	UPROPERTY(VisibleAnywhere)
	bool m_bIsFiring = false;

	UPROPERTY(VisibleAnywhere)
	bool m_bIsReloading = false;

	// 연사 타이머를 관리하기 위한 핸들
	FTimerHandle m_FireTimerHandle;

	EFireMode m_FireMode{};

	//// 클라이언트에서 전달받은 카메라 위치/회전값 캐싱
	//FVector m_CachedCameraLoc;
	//FRotator m_CachedCameraRot;

protected: /* Muzzle Awareness 관련 */

	UPROPERTY()
	AActor* m_MuzzleAwareActor{};		// Muzzle Aware 거리에 걸린 Actor (없다면 nullptr)
	FVector m_MuzzleAwareImpactPoint{};	// Muzzle Aware 거리에 걸린 ImpactPoint

protected: /* Ammo UI 업데이트 관련 */
	
	
	
private:

	// 이거 희민님이 지정한 오른손 소켓 그냥 써도 되면 그냥 쓰기
	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	static const FName s_HandSocketName;

	// 모든 무기 RifleHolster 사용처리
	static const FName s_HolsterSocketName;

	// Gun 발사 시, 카메라의 정면 방면 Trace처리할 Channel -> 모든 물체에 대한 Overlap 처리로 검사 및 Multi처리로 검사할 예정
	// TODO : 만약 Numbering 바뀌면 확인해야 함(절대적이지가 않음)
	const ECollisionChannel m_GunCrossHairTraceChannel = ECC_GameTraceChannel7;
	
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool InitializeItemActor(const FWeaponData* InRawData) override;
	

	virtual void InitializeItemData(const FWeaponData* InRawData) override;

	virtual void SwitchFireMode();

protected:
	virtual void LoadAsyncAssets(const FWeaponData* InRawData) override;

	virtual void SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo) override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	USkeletalMeshComponent* GetWeaponMesh() const { return m_WeaponMesh; }

	int32 GetCurrentAmmo() const { return m_CurrentAmmo; }

	float GetDamage() const { return m_Damage; }

	UC_AIGunUsageComponent* GetAIGunUsageComponent() const { return m_AIGunUsageComponent; }

	UParticleSystem* GetTracerFX() { return m_TracerFX; }
	
	UParticleSystem* GetImpactFX() { return m_ImpactFX; }

public:
	/// <summary>
	/// 발사 시작 동작 처리 (기본 키 : LMB Started (발사 키 클릭 이벤트 발생 시))
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : StartFire 처리가 필요없거나(이건 웬만한 무기는 다 필요할 듯), 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;

	/// <summary>
	/// 발사 동작 지속동작 처리 (기본 키 : LMB OnGoing (계속 누르고 있는 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireOnGoing 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) override;

	/// <summary>
	/// 발사 끝났을 때 처리 (기본 키 : LMB Completed (발사키 떼었을 때 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireEnd 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) override;

	/// <summary>
	/// Reload 키 동작 처리 (기본 키 R키 기능) 
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : R키에 대한 처리가 필요없거나 실패했을 경우 return false </returns>
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) override;

	virtual void UpdateAmmoInfoHUDForDrawEnd() override;

	// AI 사격 함수
	virtual void AIFire(const FVector& TargetLocation) PURE_VIRTUAL(AC_GunBase::AIFire, );

protected:
	
	/// <summary>
	/// 실질적인 사격 처리 (사격 불가능 시, return false) 
	/// </summary>
	// virtual bool PullTrigger();
	
	virtual void ReleaseTrigger();

	/// <summary>
	/// 로컬환경 자기자신 사격 실행 
	/// </summary>
	/// <returns> : 사격실패 시, return false </returns>
	virtual bool ExecuteFire();

	/// <summary>
	/// 로컬 사격 모션 재생
	/// </summary>
	/// <returns> 만약 사격 모션 재생 실패 시(Montage Priority에 의해 사격 불가능한 상황) return false </returns>
	bool PlayFireEffects();
	
	virtual void SpawnShellEject();

	// HUD 갱신
	void UpdateAmmoUI();

	/// <returns> :  실질적으로 총알이 맞은 위치 </returns>
	FVector LineTraceDamage
	(
		const FVector& CameraStart,
		const FRotator& CameraRot,
		AActor*& OutHitActor
	);

protected:
	
	// 클라이언트가 사격 후 결과를 서버로 동기화
	UFUNCTION(Server, Reliable)
	void Server_ExecuteFire(FVector_NetQuantize ImpactPoint, AActor* HitActor);

	/// <summary>
	/// <para> 클라이언트들에게 사격 연출 Multicast </para>
	/// <para> FVector_NetQuantize = 소수점 아래 아주 미세한 수치는 버리고 정수 단위 위주로 압축해서 </para>
	/// <para> 서버/클라이언트 간 주고받는 데이터 크기를 줄이는 FVector 변종 </para>
	/// <para> Unreliable -> 이 RPC 패킷이 네트워크에서 유실되어도 괜찮다는 의미 </para>
	/// <para> 이러한 시각 효과는 몇 개 패킷 유실되어도 실질적인 게임 플레이에 무방함 </para>
	/// <para> 이러한 이펙트를 UnReliable로 두는 이유는 연발발사의 경우 RPC call 빈도가 높을 수 있기 때문 </para> 
	/// </summary>
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireEffects(FVector_NetQuantize ImpactPoint);

protected:
	
	// 클라이언트가 재장전 후 결과를 서버로 동기화
	UFUNCTION(Server, Reliable)
	void Server_PlayReloadEffects();

	// 클라이언트들에게 사격 연출 Multicast
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReloadEffects();

protected:
	
	// 재장전 취소 RPC
	UFUNCTION(Server, Reliable)
	void Server_CancelReload();

	// 재생 중인 재장전 애니메이션 정지 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CancelReload();

protected:
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAIFireEffects(FVector_NetQuantize ImpactPoint);

public:
	
	/// <summary>
	/// 일반적인 Reload Animation 끝지점에서 호출처리됨 (만약 재장전 중, Animation 끊기면 해당 Notify 불리지 않는 점 인지) 
	/// </summary>
	virtual void AN_OnGunReloadEnd();

	/// <summary>
	/// 샷건류와 같이, Single 장전 처리 시 호출될 AN
	/// Shotgun에서 override해서 사용할 것
	/// </summary>
	virtual void AN_OnSingleReloadEnd() {}
	
public:

	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;
    virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;

public:
	
	/// <summary>
	/// 이 총기가 무기집으로 들어가는 처리 성공 및 들어가는 처리 시작됨
	/// 원상태 복구 처리 필요한 내역 여기서 처리
	/// </summary>
	virtual void OnSheathStart() override;

private:
	
	/// <summary>
	/// 총을 손에 쥐었을 때, 총구 앞을 가로막는 물체가 있는지에 관한 정보 전반적인 업데이트 처리
	/// 만약 총구가 파고드는 상황이라면, -> 총구가 파고드는 물체에 대해 바로 사격처리를 해버린다
	/// </summary>
	void UpdateMuzzleAwareness();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	AC_GunBase();
};
