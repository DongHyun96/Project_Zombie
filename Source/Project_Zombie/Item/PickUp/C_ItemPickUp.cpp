// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PickUp/C_ItemPickUp.h"

// Sets default values
AC_ItemPickUp::AC_ItemPickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AC_ItemPickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_ItemPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

