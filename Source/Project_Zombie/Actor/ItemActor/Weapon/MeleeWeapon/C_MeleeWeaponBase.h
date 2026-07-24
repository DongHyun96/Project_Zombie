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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components", meta = (DisplayName = "DataComponent"))
	class UC_MeleeDataTableComponent* m_DataCom;

protected:

	float						m_Damage;

	// 현재 공격 버튼을 누르고 있는 상태인지 확인
	bool						m_bIsAttack = false;

	//float						m_AttackRate;

	UPROPERTY()
	UAnimMontage*				m_PlayerAttackAnimation;

	FVector						m_PrevHitBoxSockPos;

	// TODO : 이건 무슨 구조지? 순수 궁금증 - 상연
	TSet<TWeakObjectPtr<AActor>>	m_HitActors;

protected:
	
	virtual void BeginPlay() override;
	
public:
	
	virtual void Tick(float DeltaTime) override;

public:
	
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;
	
public:
	
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;

protected:
#if WITH_EDITOR
	// 에디터에서 프로퍼티(속성)가 변경될 때마다 호출되는 엔진 함수입니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:

	virtual bool InitializeItemActor(const FWeaponData* InRawData) override;
	
	/// <summary>
	/// 멤버변수 초기화
	/// </summary>
	void Melee_init();

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

	// Hand Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HandSocketName"))
	FName m_HandSocketName{};
	
	// Holster Socket Name (각 MeleeWeapon 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HolsterSocketName"))
	FName m_HolsterSocketName{};
	
public:
	AC_MeleeWeaponBase();

};
