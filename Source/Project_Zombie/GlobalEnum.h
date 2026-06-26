#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    MAINWEAPON,
    MELEEWEAPON,
    GADGET,
    THROWABLE,
    CONSUMABLE,

};

UENUM(BlueprintType)
enum class ETeamType : uint8
{
    Player	UMETA(DisplayName = "Player"),
    Enemy	UMETA(DisplayName = "Enemy"),
    None	UMETA(DisplayName = "None"),
};