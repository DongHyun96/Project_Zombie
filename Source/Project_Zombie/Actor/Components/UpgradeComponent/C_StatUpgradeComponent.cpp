// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/UpgradeComponent/C_StatUpgradeComponent.h"


UC_StatUpgradeComponent::UC_StatUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UC_StatUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UC_StatUpgradeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

