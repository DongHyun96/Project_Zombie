// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interact/C_InteractableBase.h"

// Sets default values
AC_InteractableBase::AC_InteractableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AC_InteractableBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_InteractableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

