// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h" // 이건 뭐지?
#include "GameFramework/Actor.h"
#include "C_WeaponBase.generated.h"

UCLASS(Abstract)
class PROJECT_ZOMBIE_API AC_WeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_WeaponBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	
	/// <summary>
	/// 발사 시작 동작 처리 (기본 키 : LMB Started (발사 키 클릭 이벤트 발생 시))
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : StartFire 처리가 필요없거나(이건 웬만한 무기는 다 필요할 듯), 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnStartFire(class AC_BasicPlayer* _WeaponUser) PURE_VIRTUAL(AC_WeaponBase::OnStartFire, return false;);

	/// <summary>
	/// 발사 동작 지속동작 처리 (기본 키 : LMB OnGoing (계속 누르고 있는 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireOnGoing 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) { return false; }

	/// <summary>
	/// 발사 끝났을 때 처리 (기본 키 : LMB Completed (발사키 떼었을 때 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireEnd 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) { return false; }

	/// <summary>
	/// Reload 키 동작 처리 (기본 키 R키 기능) 
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : R키에 대한 처리가 필요없거나 실패했을 경우 return false </returns>
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) { return false; }
	
	/*virtual void StartAttack(class AC_BasicPlayer* _WeaponUser) PURE_VIRTUAL(AC_WeaponBase::StartAttack, );
	virtual void StopAttack(AC_BasicPlayer* _WeaponUser) PURE_VIRTUAL(AC_WeaponBase::StopAttack, );*/

	/// <summary>
	/// 무기집에 무기 붙이기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHolster, return false;);

	/// <summary>
	/// 손에 장착하기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHand(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHand, return false;);

public:
	
	UAnimMontage* GetDrawMontage() const { return m_DrawMontage; }
	UAnimMontage* GetSheathMontage() const { return m_SheathMontage; }
	
protected:

	// 해당 무기의 무기 꺼내는 동작 Montage
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "DrawMontage"))
	UAnimMontage* m_DrawMontage{};
	
	// 해당 무기의 무기집에 집어넣는 동작 Montage
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "SheathMontage"))
	UAnimMontage* m_SheathMontage{};


};
