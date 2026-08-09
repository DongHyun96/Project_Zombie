// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "C_MeleeWeaponBase.generated.h"


UCLASS()
class PROJECT_ZOMBIE_API AC_MeleeWeaponBase : public AC_WeaponBase
{
	GENERATED_BODY()

private:
	// private으로 내리고 DefaultsOnly를 주면 블루프린트 디테일 패널에서 내부 속성 편집이 거의 막힙니다.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* m_WeaponMesh;

protected:

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components", meta = (DisplayName = "DataComponent"))
	//class UC_MeleeDataTableComponent* m_DataCom;

protected:

	
	float						m_Damage;

	// 현재 공격 버튼을 누르고 있는 상태인지 확인
	bool						m_bIsAttack = false;

	bool						m_bSaveCombo = false;

	UPROPERTY()
	UAnimMontage*				m_PlayerAttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Effect")
	class UNiagaraSystem*		HitEffect; // 타격시 재생시킬 이펙트

	FVector						m_PrevHitBoxSockPos;

	// TODO : 이건 무슨 구조지? 순수 궁금증 - 상연
	TSet<TWeakObjectPtr<AActor>>	m_HitActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Sound")
	class USoundBase* HitSound;


protected:
	
	virtual void BeginPlay() override;
	
public:
	
	virtual void Tick(float DeltaTime) override;

public:
	
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;
	
public:
	
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;

public: // 애님 노티파이 관련

	/// <summary>
	/// 근접공격 콤보 애님 노티파이 이벤트
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Melee|AnimNotify")
	void MeleeCombo();

	/// <summary>
	/// 스킬 공격판정 활성화 시 매프레임마다 호출
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Melee|AnimNotify")
	void HitBoxCheck();

public:

	virtual bool InitializeItemActor(const FWeaponData* InRawData) override;
	
	virtual void InitializeItemData(const FWeaponData* InRawData) override;

public:
	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 (공격 시작)
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void Attack(class AC_BasicPlayer* _WeaponUser);

	/// <summary>
	/// 공격 시 캐릭터 애니메이션 재생 함수(도입예정?)
	/// <summary>
	// void PlayAttackEffects();

protected:
	// 데이터 테이블의 에셋들을 비동기 로드하기 위한 함수, 무기마다 다를 수 있기 때문에 순수 가상 함수로 선언. return 값을 bool 처리 할까?
	virtual void LoadAsyncAssets(const FWeaponData* InRawData) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	
	virtual void UpdateAmmoInfoHUDForDrawEnd() override;
	
	virtual void SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo) override;

public:
	void PlayAttackMotion(class AC_BasicPlayer* _WeaponUser);

protected:
	UFUNCTION(Server, Reliable)
	void Server_ApplyHitDamage(AActor* HitActor, float Damage, FVector ImpactPoint, FVector ImpactNormal);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitEffect(FVector ImpactPoint, FVector ImpactNormal);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAttackMotion(FName SectionName);

	UFUNCTION(Server, Reliable)
	void Server_ReqMeleeCombo();

protected:

	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HandSocketName"))
	FName m_HandSocketName{};
	
	// Holster Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HolsterSocketName"))
	FName m_HolsterSocketName{};
	

public:
	AC_MeleeWeaponBase();

};
