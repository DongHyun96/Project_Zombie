#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_EquippedComponent.generated.h"

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	NONE,
	MAIN_GUN,
	SUB_GUN,
	MELEE_WEAPON,
	THROWABLE_WEAPON,
	GADGET
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EquippedComponent : public UActorComponent
{
	GENERATED_BODY()

	// 이하 멤버 변수
protected:
	/// <summary>
	/// 슬롯 별 Weapon들, 주의 : SetSlotWeapon시에 SetActorEnableCollision(false)처리가 들어감
	/// </summary>
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<EWeaponSlot, class UC_ItemInst*> Weapons{}; // 현재 임시로 UC_ItemInst*로 선언, 나중에 UC_Weapon*으로 바꿔야할듯

	// 현재 손에 들고 있는 무기 슬롯
	UPROPERTY(VisibleAnywhere)
	EWeaponSlot CurWeaponType{};

	// 다음에 바꿀 무기 슬롯
	EWeaponSlot NextWeaponType{};

	// 이전에 들고 있던 무기 슬롯
	EWeaponSlot PrevWeaponType{};
	
	// 이하 멤버 함수 
public:	
	UC_EquippedComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Getter, Setter
public:
	// 현재 어떤 무기(슬롯)를 들고 있는지 반환.
	UFUNCTION(BlueprintCallable)
	EWeaponSlot GetCurWeaponType() const { return CurWeaponType; }

	/// <summary>
	/// 슬롯에 무기 장착하기 / 해제는 Weapon에 nullptr를 줄 것 -> 장착/해제는 이 함수를 통해서 무조건 할 것
	/// </summary>
	/// <param name="InSlot"> : 장착할 슬롯 </param>
	/// <param name="Weapon"> : 장착할 무기 객체 / 장착 해제는 nullptr </param>
	/// <returns> : 해당 slot의 이전 무기 (없었다면 return nullptr) </returns>
	//UFUNCTION(BlueprintCallable)
	//AC_Weapon* SetSlotWeapon(EWeaponSlot InSlot, class AC_Weapon* Weapon);
};
