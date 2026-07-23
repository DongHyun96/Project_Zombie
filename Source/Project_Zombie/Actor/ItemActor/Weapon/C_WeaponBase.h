// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h" // 이건 뭐지?
//#include "GameFramework/Actor.h"
//#include "Actor/ItemActor/C_ItemActor.h"
#include "GameFramework/Actor.h"
#include "C_WeaponBase.generated.h"

struct FWeaponData;
class UC_ItemLinkComponent;

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

	// 아이템 매니저에서 스폰될 때 초기화 시켜주는 함수.
	// 원래는 더 큰 구조로 잡아서 ItemActor로 모든 기능성 아이템을 포괄하고
	// const void*로 모든 데이터 테이블을 받아오려다가 우선 무기 한정으로 바꿈. -> TODO : 모든 아이템 데이터 테이블은 FItemData를 상속받으면?
	// 데이터 테이블의 Base Data + FInventoryEntry의 동적 데이터를 모두 업데이트 해야 한다.
	// 데이터 테이블은 아이템 매니저에서 매개변수로 주고, FInventoryEntry는 ItemLink를 통한다. 
	// 실제 자신의 클래스까지 내려가서 재정의해야 한다.
	virtual bool InitializeItemActor(const FWeaponData* InRawData) PURE_VIRTUAL(AC_WeaponBase::InitializeItemActor, return false;);

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
	/// Player 손에 장착하기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHand(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHand, return false;);

public:
	
	UAnimMontage* GetDrawMontage() const { return m_DrawMontage; }
	UAnimMontage* GetSheathMontage() const { return m_SheathMontage; }

public:
	
	AC_BasicPlayer* GetOwnerPlayer() const { return m_OwnerPlayer; }
	void SetOwnerPlayer(AC_BasicPlayer* _OwnerPlayer) { m_OwnerPlayer = _OwnerPlayer; }
	
	UC_ItemLinkComponent* GetLinkComp() {return ItemLinkComp;}
protected:

	// 이 Weapon을 자신의 Slot에 장착중인 OwnerPlayer
	UPROPERTY()
	AC_BasicPlayer* m_OwnerPlayer{};
	

	
protected:

	// 해당 무기의 무기 꺼내는 동작 Montage (Player character montage)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "DrawMontage"))
	UAnimMontage* m_DrawMontage{};
	
	// 해당 무기의 무기집에 집어넣는 동작 Montage (Player character montage)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "SheathMontage"))
	UAnimMontage* m_SheathMontage{};

	// 데이터 연동 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UC_ItemLinkComponent> ItemLinkComp{};
};
