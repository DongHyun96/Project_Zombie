// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interact/C_InteractableBase.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "Actor/Components/UpgradeComponent/C_ItemUpgradeComponent.h"
#include "GameModeAndManager/C_UIManager.h"

// Sets default values
AC_InteractableBase::AC_InteractableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SetReplicates(true);

	m_SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComp"));
	
	m_SphereComp->SetSphereRadius(100.f);
	
	SetRootComponent(m_SphereComp);
	
	m_MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	m_MeshComp->SetupAttachment(m_SphereComp);

	m_UpgradeComp = CreateDefaultSubobject<UC_ItemUpgradeComponent>(TEXT("ItemUpgradeComp"));


	m_InteractionComp = CreateDefaultSubobject<UC_InteractionComponent>(TEXT("InteractionComp"));

	// TODO : 여기서 해도 괜찮나? -> ㄱㅊ
	m_InteractionComp->SetInteractionNetType(EInteractionNetType::Local);

	//m_InteractionComp->SetUseTimer(false);
	m_InteractionComp->SetAllowMultipleInteractor(true);

}

// Called when the game starts or when spawned
void AC_InteractableBase::BeginPlay()
{
	Super::BeginPlay();
	m_InteractionComp->SetupInteraction(m_SphereComp);
}

// Called every frame
void AC_InteractableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_InteractableBase::RequestItemUpgrade(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat)
{
	if (!m_UpgradeComp) return;

	PRINT_LOCAL(GetWorld(), "RequestItemUpgrade", FColor::Blue, 5.f);

	m_UpgradeComp->UpgradeItem(InPlayer, InItemIndex, TargetStat);
}
