#include "Actor/Components/C_EquippedComponent.h"

UC_EquippedComponent::UC_EquippedComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UC_EquippedComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UC_EquippedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

