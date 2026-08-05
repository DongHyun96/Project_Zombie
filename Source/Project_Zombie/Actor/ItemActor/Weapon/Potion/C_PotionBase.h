// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "C_PotionBase.generated.h"

class UCapsuleComponent;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_PotionBase : public AC_WeaponBase
{
	GENERATED_BODY()
	
public:
	AC_PotionBase();
	
	// UsingMontage에서 호출될 실제 기능을 하는  Anim Notify 함수. 
	// 서버와 사용자만 알고 있으면 된다. 혹은 한쪽만 알고 나중에 알려주는 방식.
	UFUNCTION(BlueprintCallable, Category = "Potion|AnimNotify")
	void OnAction();
public:
	virtual bool InitializeItemActor(const FWeaponData* InRawData) override;

	virtual void InitializeItemData(const FWeaponData* InRawData) override;
	
protected:
	virtual void LoadAsyncAssets(const FWeaponData* InRawData) override;
	
	
	/// 급해서 그냥 Weapon 로직 이용해서 Potion 제작.
public:
	/// <summary>
	/// 발사 시작 동작 처리 (기본 키 : LMB Started (발사 키 클릭 이벤트 발생 시))

	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : StartFire 처리가 필요없거나(이건 웬만한 무기는 다 필요할 듯), 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnStartFire(class AC_BasicPlayer* _WeaponUser) override;
	
	/// <summary>
	/// 무기집에 무기 붙이기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;

	/// <summary>
	/// Player 손에 장착하기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;
	
public:
	/// <summary>
	/// 현재 무기 상태에 맞춘 AmmoUIInfo 초기화 처리 
	/// </summary>
	/// <param name="_AmmoUIInfo"></param>
	virtual void SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo) override;
	
	/// <summary>
	/// <para> DrawEnd 시, 각 무기에 맞는 표기할 무기 정보 띄우기 처리 (Local Player에 한해 처리) </para>
	/// <para> 이 무기의 주인이 Locally Controlled 되는 중인지 체킹하여 valid하면 띄움 </para>
	/// <para> 실질적인 DrawEnd 시에 호출 및 리슨서버 환경에서 EquippedCom에서 현재 들고 있는 무기가 바뀌는 Rep 처리 시, </para>
	/// <para> 해당 함수를 이용할 예정 </para>
	/// </summary>
	virtual void UpdateAmmoInfoHUDForDrawEnd() override;
	
	// ThrowableBase에서 구조 가져와서 사용.
private:
	// 로컬에서 즉시 몽타주 재생하고 서버에 동기화 요청
	void PlayUsingMontageSynced();

	UFUNCTION(Server, Reliable)
	void Server_PlayUsingMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayUsingMontage();
	
	UFUNCTION(Server, Reliable)
	void Server_DecreaseCurCount();	
	
private:
	// Holster(무기집 위치) Socket Name (모든 Throwable 공통 무기집 위치 사용할 예정)
	static const FName s_HolsterSocketName;
	
protected:
	// Hand Socket Name (각 Potion 블루프린트에서 Name 초기화 해줄 것)
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "HandSocketName"))
	FName m_HandSocketName{};
protected:
	// HP 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Potion_value = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LeftTotalCount = 0;
	
	// Collision이 필요한가?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCapsuleComponent* capsuleComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent = nullptr;

	// 좌클릭시 사용을 시작 할 때부터 재생할 몽타주
	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> m_UsingAnimation{};  
};
