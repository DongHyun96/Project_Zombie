// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieStatComponent.h"
#include "C_ZombieStatData.h"

UC_ZombieStatComponent::UC_ZombieStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UC_ZombieStatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UC_ZombieStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

