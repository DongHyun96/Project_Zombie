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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class USphereComponent* m_Collision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Mesh", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* m_WeaponMesh;		// 정적정보 - 상연, 데이터 테이블에서 가져와서 초기화 해주기.

	// AI Enemy가 Gun을 사용하는 처리 기능 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (DisplayName = "AIGunUsageComponent"))
	UC_AIGunUsageComponent* m_AIGunUsageComponent{};
	
protected:
	// 총이 갖는 최종 데미지
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	float					m_Damage;

	// 현재 남아있는 총알 수
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Stats")
	int32 m_CurrentAmmo;

	// 총이 갖는 최종 MaxAmmo
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	int32					m_MaxAmmo;

	// 총이 갖는 최종 FireRate
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	float					m_FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GunStats")
	float m_SpreadAngle = 0.0f;
	
	class UAnimMontage* m_PlayerFireAnimation;

	class UAnimMontage* m_PlayerReloadAnimation;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimSequence> m_FireAnimation;   		// 정적정보. - 상연, 데이터 테이블에서 가져와서 초기화 해주기.
	
	UPROPERTY(Transient)
	TObjectPtr<class UAnimSequence> m_ReloadAnimation;		// 정적정보. - 상연, 데이터 테이블에서 가져와서 초기화 해주기.
	
	UPROPERTY(Transient)
	TObjectPtr<class UStaticMesh> m_ShellMesh;				// 정적정보. - 상연, 데이터 테이블에서 가져와서 초기화 해주기.

	// 이 무기의 원본 데이터 및 동적 데이터(CustomData)를 보유하는 통합 Entry
	// TODO : Lagacy로 지워질 예정
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	//FInventoryEntry			ItemEntry;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UNiagaraSystem* m_ShellEjectNiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	class UParticleSystem* m_TracerFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	class UParticleSystem* m_ImpactFX;

protected:
	// 현재 사격 버튼을 누르고 있는 상태인지 확인
	bool m_bIsFiring = false;

	// 현재 재장전 상태인지 확인
	UPROPERTY(Replicated)
	bool m_bIsReloading = false;

	// 연사 타이머를 관리하기 위한 핸들
	FTimerHandle m_FireTimerHandle;

	FTimerHandle m_ReloadTimerHandle;

	EFireMode m_FireMode{};

	//// 클라이언트에서 전달받은 카메라 위치/회전값 캐싱
	//FVector m_CachedCameraLoc;
	//FRotator m_CachedCameraRot;

private:

	// 이거 희민님이 지정한 오른손 소켓 그냥 써도 되면 그냥 쓰기
	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	static const FName s_HandSocketName;

	// 모든 무기 RifleHolster 사용처리
	static const FName s_HolsterSocketName;

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool InitializeItemActor(const FWeaponData* InRawData) override;
	
	virtual void SwitchFireMode();

protected:
	virtual void LoadAsyncAssets(const FWeaponData* InRawData) override;

	virtual void SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo) override;
	
public:
	USkeletalMeshComponent* GetWeaponMesh() const { return m_WeaponMesh; }

	int32 GetCurrentAmmo() const { return m_CurrentAmmo; }

	UC_AIGunUsageComponent* GetAIGunUsageComponent() const { return m_AIGunUsageComponent; }
	
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

protected:
	virtual void PullTrigger();
	virtual void ReleaseTrigger();

	// 클라이언트 사격 실행
	virtual void Client_ExecuteFire();

	// 로컬 사격/재장전
	virtual void PlayFireEffects_Client();
	virtual void SpawnShellEject();

	// HUD 갱신
	void UpdateAmmoUI();

	virtual FVector LineTraceDamage(const FVector& CameraStart, const FRotator& CameraRot, AActor*& OutHitActor);

protected:
	// 클라이언트가 사격 후 결과를 서버로 동기화
	UFUNCTION(Server, Reliable)
	void Server_ExecuteFire(FVector_NetQuantize ImpactPoint, AActor* HitActor);

	// 클라이언트들에게 사격 연출 Multicast
	// FVector_NetQuantize = 소수점 아래 아주 미세한 수치는 버리고 정수 단위 위주로 압축해서 
	// 서버/클라이언트 간 주고받는 데이터 크기를 줄이는 FVector 변종
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireEffects(FVector_NetQuantize ImpactPoint);

	// 클라이언트가 재장전
	UFUNCTION(Client, Reliable)
	void Client_CompleteReload();

	// 클라이언트가 재장전 후 결과를 서버로 동기화
	UFUNCTION(Server, Reliable)
	void Server_StartReload();

	// 클라이언트들에게 사격 연출 Multicast
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReloadEffects();

	// 재장전 취소 RPC
	UFUNCTION(Server, Reliable)
	void Server_CancelReload();

	// 재생 중인 재장전 애니메이션 정지 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopReloadEffects();

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
	
	UFUNCTION()
	void OnMainColliderBeginOverlap
	(
		UPrimitiveComponent* _OverlapComponent,
		AActor*				 _OtherActor,
		UPrimitiveComponent* _OtherComp,
		int32				 _OtherBodyIndex,
		bool				 _bFromSweep,
		const FHitResult&	 _SweepResult
	);
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	AC_GunBase();
};
