#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "TaskSyncManager.h"
#include "C_EquippedComponent.generated.h"

class AC_BasicPlayer;
enum class EFireMode : uint8;
class UC_InvenComponent;

USTRUCT(BlueprintType)
struct FAmmoUIInfo
{
	GENERATED_BODY()

	UPROPERTY()
	bool		Visible{};
	
	UPROPERTY()
	EFireMode	FireMode{};
	
	UPROPERTY()
	int32 		MagazineAmmo{};
	
	UPROPERTY()
	int32 		LeftAmmoTotalCount{};
};

/// <summary>
/// 장착된 무기 관리 및 무기전환, 현재 손에 들고 있는 무기 관리 처리 Component
/// </summary>
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EquippedComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UC_EquippedComponent();
	
	// OwnerPlayer Setter가 안보여서 급히 만듬.
	void SetOwnerPlayer(AC_BasicPlayer* _Player) { m_OwnerPlayer = _Player; }

protected:

	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable)
	class AC_WeaponBase* GetCurWeapon() const { return m_Weapons[m_CurWeaponTypeIdx]; }

	/// <summary>
	/// 해당 Slot에 장착된 Weapon 구하기 (만약 장착된 무기가 없다면 nullptr 
	/// </summary>
	AC_WeaponBase* GetSlotWeapon(EWeaponSlot _WeaponSlotType) const { return m_Weapons[static_cast<uint8>(_WeaponSlotType)]; }
	
public:
	
	/// <summary>
	/// 서버 쪽에 SetSlotWeapon 알림 도착 -> 해당 Slot의 무기 WeaponToEquip로 갈아끼우기 
	/// </summary>
	UFUNCTION(Server, Reliable)
	void Server_SetSlotWeapon(EWeaponSlot _TargetSlot, AC_WeaponBase* _WeaponToEquip);
	
private:
	
	/// <summary>
	/// 슬롯에 무기 장착하기 / 해제는 Weapon에 nullptr를 줄 것 -> 장착/해제는 이 함수를 통해서 무조건 할 것
	/// </summary>
	/// <param name="TargetSlot"> : 장착할 슬롯 위치 </param>
	/// <param name="WeaponToEquip"> : 해당 slot에 장착할 무기 객체 / 장착 해제는 nullptr </param>
	void SetSlotWeapon(EWeaponSlot TargetSlot, AC_WeaponBase* WeaponToEquip);

public:
	
	/// <summary>
	/// 서버에게 실제 장착하는 아이템 스폰 요청.
	/// </summary>
	/// <param name="SlotIndex"> : Player의 인벤토리의 장비전용 슬롯 인덱스</param>
	/// <param name="ItemData"> : 아이템 데이터</param>
	UFUNCTION(Server, Reliable)
	void Server_RequestSpawnEquippedActor(int32 SlotIndex, const FInventoryEntry& ItemData);

	
	// 아이템 업그레이드시 장착중인 아이템에 즉시 적용하게 업데이트 하는 함수.
	void UpdateWeaponData(EWeaponSlot _TargetWeapon, FName InItemRow);
	
	UFUNCTION(Client, Reliable)
	void Client_UpdateWeaponData(EWeaponSlot _TargetWeapon, FName InItemRow);
public:
	
	/// <summary>
	/// 현재 손에 든 무기 바꾸기 (LOCAL 환경에서 자기 player는 바로 처리할 것)
	/// </summary>
	/// <param name="_ChangeTo"> : 새로이 바꿔서 들려고 하는 무기 슬롯 종류 </param>
	/// <returns> 바꾸기 성공했다면 return true </returns>
	bool ChangeCurWeapon(EWeaponSlot _ChangeTo);

	/// <summary>
	/// X키를 통한 무기 집어넣기 및 직전 무기 꺼내기 (LOCAL에서 자기 player는 바로 처리할 것)
	/// </summary>
	/// <returns> : 실패 시 return false </returns>
	bool ToggleArmed();
	
public:
	// 인벤토리 컴포넌트를 전달받아 델리게이트 바인딩
	void SetupInventoryComponent(UC_InvenComponent* InInvenComp);

	// 연결 해제
	void ClearInventoryComponent();
	
	void LoadEquippedWeaponFromInven(int32 SlotIndex, const FInventoryEntry& ItemData);
	
protected:
	UFUNCTION()
	void OnInventorySlotChanged(int32 SlotIndex, const FInventoryEntry& ItemData);
	
public:
	
	/// <summary>
	/// <para> 무기 Sheath가 끝났을 시 Notify로 불러질 call back 함수 </para>
	/// <para> 상황에 따라 다음 무기 꺼내는 동작 처리를 진행한다(Most trivial case) </para>
	/// <para> 서버 쪽 캐릭터만 해당 AN 처리 판단을 한다 </para> 
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void OnSheathEnd();

	/// <summary>
	/// 무기 Draw가 끝났을 시 Notify로 불러질 call back 함수
	/// </summary>
	/// <para> 서버 쪽 캐릭터만 해당 AN 처리 판단을 한다 </para>
	UFUNCTION(BlueprintCallable)
	void OnDrawEnd();

private:
	
	/// <summary>
	/// 현재 EquippedComponent 상태에 맞추어 AmmoWidget 정보 업데이트 
	/// </summary>
	void UpdateAmmoWidget();

	/// <summary>
	/// SetSlotWeapon 처리 시, AmmoWidget 업데이트 이 Client RPC Call을 통해 처리할 것 
	/// </summary>
	UFUNCTION(Client, Reliable)
	void Client_UpdateAmmoWidget(const FAmmoUIInfo& _AmmoUIInfo);
	
private:
	
	/// <summary>
	/// 로컬 쪽에서 PlayDrawMontage 발생 -> 서버 쪽으로 멀티캐스트 부탁(다른 환경에서도 DrawMontage 재생 처리를 하라고)
	/// </summary>
	UFUNCTION(Server, Reliable)
	void Server_PlayDrawMontage(AC_WeaponBase* _TargetWeapon);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDrawMontage(AC_WeaponBase* _TargetWeapon);

	UFUNCTION(Server, Reliable)
	void Server_PlaySheathMontage(AC_WeaponBase* _TargetWeapon);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySheathMontage(AC_WeaponBase* _TargetWeapon);

private:
	
	/// <summary>
	/// 서버 쪽 무기 AttachToHolster 처리 요청 처리
	/// </summary>
	UFUNCTION(Server, Reliable)
	void Server_AttachToHolster(AC_WeaponBase* _TargetWeapon);

	/// <summary>
	/// 서버 쪽 무기 AttachToHand 처리 요청 처리 
	/// </summary>
	UFUNCTION(Server, Reliable)
	void Server_AttachToHand(AC_WeaponBase* _TargetWeapon);
	
	UFUNCTION(Server, Reliable)
	void Server_SetCurWeaponIdx(uint8 _NewIdx);
	
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private: /* This component owner Player */

	UPROPERTY()
	class AC_BasicPlayer* m_OwnerPlayer{};
	
protected: /* 장착 무기 Slot */  
	
	// 현재 슬롯 별 장착된 Weapon들 (각 EWeaponSlot Type 자리는, 각 index)
	// 장착된 무기가 없는 슬롯은 nullptr가 들어간다
	/*UPROPERTY(ReplicatedUsing = , VisibleAnywhere, BlueprintReadOnly, DisplayName =  "Weapons")*/
	// UPROPERTY(ReplicatedUsing = OnRep_Weapons, VisibleAnywhere, BlueprintReadOnly, DisplayName =  "Weapons")
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, DisplayName =  "Weapons")
	TArray<AC_WeaponBase*> m_Weapons{};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TMap<EWeaponSlot, TSubclassOf<AC_WeaponBase>> m_WeaponClassToSpawn{};

	// 가비지 컬렉션을 방해하지 않는 약한 참조 (메모리 부담 전혀 없음)
	TWeakObjectPtr<UC_InvenComponent> BoundInvenComp{};

protected:

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	uint8 m_CurWeaponTypeIdx  = static_cast<uint8>(EWeaponSlot::None); // 현재 손에 들고 있는 무기 슬롯 Type Idx

	UPROPERTY()
	uint8 m_NextWeaponTypeIdx = static_cast<uint8>(EWeaponSlot::None); // 다음에 바꿀 무기 슬롯
	
	UPROPERTY()
	uint8 m_PrevWeaponTypeIdx = static_cast<uint8>(EWeaponSlot::None); // 이전에 들고 있던 무기 슬롯
		

	
	
};
