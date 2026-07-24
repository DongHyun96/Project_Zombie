#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    MAINWEAPON,
    MELEEWEAPON,
    THROWABLE,
    GADGET,
    CONSUMABLE,
    MATTER,
    MAX,
};

UENUM(BlueprintType)
enum class ETeamType : uint8
{
    Player	UMETA(DisplayName = "Player"),
    Enemy	UMETA(DisplayName = "Enemy"),
    None	UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Gun         UMETA(DisplayName = "Gun"),
    Melee       UMETA(DisplayName = "Melee"),
    Throwable   UMETA(DisplayName = "Throwable"),
    Max
};

// 아이템 데이터 테이블에 접근할 때 사용할 아이템 타입 enum
UENUM(BlueprintType)
enum class EItemTableType : uint8
{
    General,
    Gun,
    Melee,
    Throwable
};

// GlobalEnum의 EWeaponType으로 통합하는 방향으로 가면 될 듯?
// 다만 InvenComponent에서 enum값으로 장비칸을 추가적으로 만들고 있는데, max를 없애고 NONE을 마지막에 두면 EWeaponType과 EWeaponSlot을 통합시키고
// 사용에도 용의할 듯.
UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
    MainWeapon,
    MeleeWeapon,
    ThrowableWeapon,
    //Gadget,							// 나중에 추가할지도 모르는 장비 슬롯 (예: 방어구, 액세서리, 설치형 무기등), gadget : 간단한 기계 장치
    None,
    Max				UMETA(Hidden)
};

// 투척류 타입
UENUM(BlueprintType)
enum class EThrowableType : uint8
{
	None,
	Grenade,
	Molotov,
};

// 투척류 상태
UENUM(BlueprintType)
enum class EThrowableState : uint8
{
	Idle,		// 기본 상태
	RemovePin,	// 핀 제거
	Ready,		// 투척 준비 
	ReadyLoop,	// 투척 준비 동작 루프
	Throwing,	// 투척 중
	Thrown,		// 투척 
	Exploded,	// 폭발 
};
