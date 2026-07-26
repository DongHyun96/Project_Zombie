#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_Interactable.generated.h"

class UC_InteractionComponent;

UINTERFACE(MinimalAPI)
class UI_Interactable : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_ZOMBIE_API II_Interactable
{
	GENERATED_BODY()

public:
	virtual UC_InteractionComponent* GetInteractionComponent() const = 0;
};
