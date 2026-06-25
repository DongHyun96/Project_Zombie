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