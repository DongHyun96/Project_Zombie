// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NormalZombie.h"

#include "Components/BoxComponent.h"


AC_NormalZombie::AC_NormalZombie()
	: Super(EZombieType::NormalZombie)
{
	PrimaryActorTick.bCanEverTick = false;
	
	m_NormalAttackCollider = CreateDefaultSubobject<UBoxComponent>("NormalAttackCollider");
	m_NormalAttackCollider->SetupAttachment(GetMesh(), TEXT("RightArm"));
	AddNormalAttackCollider(m_NormalAttackCollider);
}

void AC_NormalZombie::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocallyControlled())
		m_NormalAttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AC_NormalZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
