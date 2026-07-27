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

	// TODO : Lagacy로 삭제 예정
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components", meta = (DisplayName = "DataComponent"))
	//class UC_GunDataTableComponent* m_DataCom;

	// AI Enemy가 Gun을 사용하는 처리 기능 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (DisplayName = "AIGunUsageComponent"))
	UC_AIGunUsageComponent* m_AIGunUsageComponent{};
	
protected:
	// 총이 갖는 최종 데미지
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	float					m_Damage;

	// 현재 남아있는 총알 수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	int32					m_CurrentAmmo;

	// 총이 갖는 최종 MaxAmmo
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	int32					m_MaxAmmo;

	// 총이 갖는 최종 FireRate
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|Stats")
	float					m_FireRate;

	// 총이 갖는 최종 ShellEjectImpulse(탄피 배출에 가하는 힘), TODO : 아마 나중에 Actor로 만든게 아니라 나이아가라등으로 바꾸면서 사라질 수 도? 
	float					m_ShellEjectImpulse;	// 살짝 애매 - 상연
	
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

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shell")
	TObjectPtr<class UNiagaraSystem> m_ShellEjectNiagaraSystem;

protected:
	// 현재 사격 버튼을 누르고 있는 상태인지 확인
	bool m_bIsFiring = false;

	// 현재 재장전 상태인지 확인
	bool m_bIsReloading = false;

	// 연사 타이머를 관리하기 위한 핸들
	FTimerHandle m_FireTimerHandle;

protected:
	
	// 이 총기의 FireMode 종류 (한 종류로만 설정되도록 정해짐)
	EFireMode m_FireMode{};
	
private:

	// 이거 희민님이 지정한 오른손 소켓 그냥 써도 되면 그냥 쓰기
	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	static const FName s_HandSocketName;

protected:
#if WITH_EDITOR
	// 에디터에서 프로퍼티(속성)가 변경될 때마다 호출되는 엔진 함수. TODO : 이제 init함수가 바뀌어서 사용하지 않음.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Holster Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	// 이거는 무기마다 Socket Transform 다를 수 있다고 판단됨 
	// TODO : 블루프린트쪽에서만 넣어주고 있기 때문에 블프에서 빠뜨리거나 블프로 안만든 클래스를 스폰하면 m_HolsterSocketName이 비어있음.
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HolsterSocketName"))
	FName m_HolsterSocketName{}; // 그렇다면 DataTable에서 관리해도 될듯? 정적 정보로 판단. - 상연

public:
	
	/// <summary>
	/// 멤버변수 초기화,
	/// TODO : 삭제하거나 아이템 매니저에서 데이터 테이블 받아와서 처리해야함. 개인적으로는 지우는게 나을 듯. 
	/// </summary>
	//void Gun_init();

	/// <summary>
	/// 탄약 체크 및 UI 업데이트 (사격 가능하면 true 반환)
	/// </summary>
	bool ConsumeAmmo();   

	/// <summary>
	/// 공통 탄피 배출 로직
	/// </summary>
	void SpawnShellEject();
	
	// 리팩토링중....
public:
	// 인벤토리나 Pickup에서 Spawn/Attach 시 호출하여 데이터를 주입, TODO : ItemActor를 상속받게 하고 ItemActor에서 선언하기.
	//virtual void InitFromInventoryEntry(const FInventoryEntry& InEntry);

	// 액터에서 수정된 동적 정보(남은 탄약, 업그레이드 등)를 반영한 최신 Entry 반환, TODO : ItemActor를 상속받게 하고 ItemActor에서 선언하기.
	//virtual FInventoryEntry GetUpdatedInventoryEntry();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool InitializeItemActor(const FWeaponData* InRawData) override;
	
	virtual void SwitchFireMode();

protected:
	virtual void LoadAsyncAssets(const FWeaponData* InRawData) override;
	
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

public:
	
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;
    virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;
	
	virtual void PullTrigger() {}
	virtual void ReleaseTrigger() {}

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
