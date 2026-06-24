#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_EquippedComponent.generated.h"

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	None,
	MainWeapon,
	MeleeWeapon,
	ThrowableWeapon,
	Gadget,							// 나중에 추가할지도 모르는 장비 슬롯 (예: 방어구, 액세서리, 설치형 무기등), gadget : 간단한 기계 장치
	Max				UMETA(Hidden)
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EquippedComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UC_EquippedComponent();

protected:
	virtual void BeginPlay() override;

public:
	
	/// <summary>
	/// 슬롯에 무기 장착하기 / 해제는 Weapon에 nullptr를 줄 것 -> 장착/해제는 이 함수를 통해서 무조건 할 것
	/// </summary>
	/// <param name="TargetSlot"> : 장착할 슬롯 위치 </param>
	/// <param name="WeaponToEquip"> : 해당 slot에 장착할 무기 객체 / 장착 해제는 nullptr </param>
	/// <returns> : 해당 slot의 이전 무기 (없었다면 return nullptr) </returns>
	class AC_WeaponBase* SetSlotWeapon(EWeaponSlot TargetSlot, AC_WeaponBase* WeaponToEquip);

	/// <summary>
	/// 현재 손에 든 무기 바꾸기
	/// </summary>
	/// <param name="_ChangeTo"> : 새로이 바꿔서 들려고 하는 무기 슬롯 종류 </param>
	/// <returns> 바꾸기 성공했다면 return true </returns>
	bool ChangeCurWeapon(EWeaponSlot _ChangeTo);
	
private:
	
	class AC_BasicPlayer* m_OwnerPlayer{};
	
protected:
	
	// 현재 슬롯 별 장착된 Weapon들 (각 EWeaponSlot Type 자리는, 각 index)
	// 장착된 무기가 없는 슬롯은 nullptr가 들어간다
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<AC_WeaponBase*> m_Weapons{};
	
	// 현재 손에 들고 있는 무기 슬롯 Type
	UPROPERTY(VisibleAnywhere)
	EWeaponSlot CurWeaponType{};

private:
	
	// 다음에 바꿀 무기 슬롯
	EWeaponSlot NextWeaponType{};

	// 이전에 들고 있던 무기 슬롯
	EWeaponSlot PrevWeaponType{};
	
};
