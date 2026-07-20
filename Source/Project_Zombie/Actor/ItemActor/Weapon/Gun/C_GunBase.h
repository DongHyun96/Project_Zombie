// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class USphereComponent* m_Collision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Mesh", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* m_WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components", meta = (DisplayName = "DataComponent"))
	class UC_GunDataTableComponent* m_DataCom;

protected:
	float					m_BaseDamage;

	//현재 남아있는 총알 수
	int32					m_CurrentAmmo;

	int32					m_MaxAmmo;

	float					m_FireRate;

	float					m_ShellEjectImpulse;

	class UAnimSequence*	m_FireAnimation;

	class UAnimSequence*	m_ReloadAnimation;

	class UStaticMesh*		m_ShellMesh;

	
protected:
	// 현재 이 Gun을 사용중인 WeaponUser (Player)
	UPROPERTY()
	AC_BasicPlayer* m_WeaponPlayerUser{};
	
	// 현재 이 Gun을 사용중인 CopZombieUser
	UPROPERTY()
	class AC_CopZombie* m_WeaponCopZombieUser{};
	
private:

	// 이거 희민님이 지정한 오른손 소켓 그냥 써도 되면 그냥 쓰기
	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	static const FName s_HandSocketName;

protected:
#if WITH_EDITOR
	// 에디터에서 프로퍼티(속성)가 변경될 때마다 호출되는 엔진 함수.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Holster Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	// 이거는 무기마다 Socket Transform 다를 수 있다고 판단됨
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HolsterSocketName"))
	FName m_HolsterSocketName{};

public:

	/// <summary>
	/// 멤버변수 초기화
	/// </summary>
	void Gun_init();

	/// <summary>
	/// 탄약 체크 및 UI 업데이트 (사격 가능하면 true 반환)
	/// </summary>
	bool ConsumeAmmo();   

	/// <summary>
	/// 공통 탄피 배출 로직
	/// </summary>
	void SpawnShellEject();

	/// <summary>
	/// 공통 라인트레이스 데미지 처리
	/// </summary>
	void ProcessLineTraceDamage(float DamageVal);

public:
	class USkeletalMeshComponent* GetWeaponMesh() { return m_WeaponMesh; }

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
	
public:
	
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;
	virtual bool AttachToEnemyHand(USceneComponent* _ParentMesh) override;
	
    virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;
	
	virtual void PullTrigger() {}
	virtual void ReleaseTrigger() {}


protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	AC_GunBase();
};
