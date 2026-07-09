// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ItemActor/C_ItemActor.h"

// Sets default values
AC_ItemActor::AC_ItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AC_ItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_ItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

