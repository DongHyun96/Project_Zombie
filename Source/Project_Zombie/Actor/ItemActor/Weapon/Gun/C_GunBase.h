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

private:
	// private으로 내리고 DefaultsOnly를 주면 블루프린트 디테일 패널에서 내부 속성 편집이 거의 막힙니다.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* m_WeaponMesh;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components", meta = (DisplayName = "DataComponent"))
	class UC_GunDataTableComponent* m_DataCom;

protected:
	// 현재 사격 버튼을 누르고 있는 상태인지 확인
	bool					m_bIsFiring = false;

	//현재 남아있는 총알 수
	int32					m_CurrentAmmo;

	int32					m_MaxAmmo;

	float					m_FireRate;

	float					m_ShellEjectImpulse;

	// 연사 타이머를 관리하기 위한 핸들
	FTimerHandle			m_FireTimerHandle;

	UAnimSequence*			m_FireAnimation;

	UAnimSequence*			m_ReloadAnimation;

	UStaticMesh*			m_ShellMesh;

	

private:

	// 이거 희민님이 지정한 오른손 소켓 그냥 써도 되면 그냥 쓰기
	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	static const FName s_HandSocketName;

protected:
	
	// 에디터에서 프로퍼티(속성)가 변경될 때마다 호출되는 엔진 함수입니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

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
	/// 마우스 왼쪽 버튼 클릭 (사격 시작)
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void PullTrigger();

	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 뗌 (사격 중지)
	/// <summary>
	UFUNCTION(BlueprintCallable)
	void ReleaseTrigger();

	/// <summary>
	/// R 키 입력 (재장전 요청)
	/// <summary>
	UFUNCTION(BlueprintCallable)
	void Gun_Reload();

	/// <summary>
	/// 방아쇠를 당겼을 때 애니메이션 재생 함수
	/// <summary>
	UFUNCTION(BlueprintCallable)
	void CompleteReload();

	void PlayFireEffects();

public:
	USkeletalMeshComponent* GetWeaponMesh() { return m_WeaponMesh; }

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
    virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	AC_GunBase();
};
