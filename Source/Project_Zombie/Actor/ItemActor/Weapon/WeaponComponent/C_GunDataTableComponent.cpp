// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunDataTableComponent.h"

UC_GunDataTableComponent::UC_GunDataTableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UC_GunDataTableComponent::InitGunDataFromStruct(UScriptStruct* _InStruct, const void* _StrctPtr)
{
}

void UC_GunDataTableComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UC_GunDataTableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

