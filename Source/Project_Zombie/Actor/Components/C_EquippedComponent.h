#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "C_EquippedComponent.generated.h"

class UC_InvenComponent;

/// <summary>
/// 장착된 무기 관리 및 무기전환, 현재 손에 들고 있는 무기 관리 처리 Component
/// </summary>
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EquippedComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UC_EquippedComponent();

protected:
	virtual void BeginPlay() override;

public:

	class AC_WeaponBase* GetCurWeapon() const { return m_Weapons[m_CurWeaponTypeIdx]; }

	/// <summary>
	/// 해당 Slot에 장착된 Weapon 구하기 (만약 장착된 무기가 없다면 nullptr 
	/// </summary>
	AC_WeaponBase* GetSlotWeapon(EWeaponSlot _WeaponSlotType) const { return m_Weapons[static_cast<uint8>(_WeaponSlotType)]; }
	
	/// <summary>
	/// 슬롯에 무기 장착하기 / 해제는 Weapon에 nullptr를 줄 것 -> 장착/해제는 이 함수를 통해서 무조건 할 것
	/// </summary>
	/// <param name="TargetSlot"> : 장착할 슬롯 위치 </param>
	/// <param name="WeaponToEquip"> : 해당 slot에 장착할 무기 객체 / 장착 해제는 nullptr </param>
	/// <returns> : 해당 slot의 이전 무기 (없었다면 return nullptr) </returns>
	AC_WeaponBase* SetSlotWeapon(EWeaponSlot TargetSlot, AC_WeaponBase* WeaponToEquip);

	
public:	
	/// <summary>
	/// 현재 손에 든 무기 바꾸기
	/// </summary>
	/// <param name="_ChangeTo"> : 새로이 바꿔서 들려고 하는 무기 슬롯 종류 </param>
	/// <returns> 바꾸기 성공했다면 return true </returns>
	bool ChangeCurWeapon(EWeaponSlot _ChangeTo);

	/// <summary>
	/// X키를 통한 무기 집어넣기 및 직전 무기 꺼내기
	/// </summary>
	/// <returns> : 실패 시 return false </returns>
	bool ToggleArmed();


	
public:
	
	/// <summary>
	/// <para> 무기 Sheath가 끝났을 시 Notify로 불러질 call back 함수 </para>
	/// <para> 상황에 따라 다음 무기 꺼내는 동작 처리를 진행한다(Most trivial case) </para>
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void OnSheathEnd();

	/// <summary>
	/// 무기 Draw가 끝났을 시 Notify로 불러질 call back 함수
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void OnDrawEnd();
	
private: /* This component owner Player */

	UPROPERTY()
	class AC_BasicPlayer* m_OwnerPlayer{};
	
protected: /* 장착 무기 Slot */  
	
	// 현재 슬롯 별 장착된 Weapon들 (각 EWeaponSlot Type 자리는, 각 index)
	// 장착된 무기가 없는 슬롯은 nullptr가 들어간다
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AC_WeaponBase*> m_Weapons{};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TMap<EWeaponSlot, TSubclassOf<AC_WeaponBase>> m_WeaponClassToSpawn{};

	
private:

	// 현재 무기를 바꾸는 과정에 
	bool m_bIsCurrentlyChangingWeapon{};
	
	uint8 m_CurWeaponTypeIdx = static_cast<uint8>(EWeaponSlot::None); // 현재 손에 들고 있는 무기 슬롯 Type Idx
	uint8 m_NextWeaponTypeIdx = static_cast<uint8>(EWeaponSlot::None); // 다음에 바꿀 무기 슬롯
	uint8 m_PrevWeaponTypeIdx = static_cast<uint8>(EWeaponSlot::None); // 이전에 들고 있던 무기 슬롯
		

	
	
};
