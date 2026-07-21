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
    Throwable   UMETA(DisplayName = "Throwable")
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