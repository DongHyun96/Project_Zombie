// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interact/C_InteractableBase.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
AC_InteractableBase::AC_InteractableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = m_MeshComp;

	m_SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComp"));
	m_SphereComp->SetSphereRadius(100.f);
	m_SphereComp->SetupAttachment(RootComponent);

	m_InteractionComp = CreateDefaultSubobject<UC_InteractionComponent>(TEXT("InteractionComp"));

	m_InteractionComp->SetupInteraction(m_SphereComp);

	// TODO : 여기서 해도 괜찮나?
	m_InteractionComp->SetInteractionNetType(EInteractionNetType::Local);

	m_InteractionComp->SetUseTimer(false);
	m_InteractionComp->SetAllowMultipleInteractor(true);

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

