// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopZombie.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/BoxComponent.h"
#include "Utility/C_Util.h"


AC_CopZombie::AC_CopZombie()
	: Super(EZombieType::CopZombie)
{
	PrimaryActorTick.bCanEverTick = false;
	
	m_GrabRangeCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("GrabRangeCollider"));
	m_GrabRangeCollider->SetupAttachment(GetRootComponent());
}

void AC_CopZombie::BeginPlay()
{
	Super::BeginPlay();
	// m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	m_GrabRangeCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_CopZombie::OnGrabRangeColliderBeginOverlap);
	m_GrabRangeCollider->OnComponentEndOverlap.AddDynamic(this, &AC_CopZombie::OnGrabRangeColliderEndOverlap);
}

void AC_CopZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_CopZombie::OnANSGrabStart()
{
	m_GrabRangeEnteredPlayers.Empty();
	m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AC_CopZombie::OnANSGrabEnd()
{
	m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// ANS가 끊기는 것이 -> Abort Task에 의해 끊길 수도 있음 -> 따라서 자체적으로 BTTask_GrabMainWeapon에서 EndSkill 시, Skill 성공 여부에 따라서 처리해줄 것
}

void AC_CopZombie::OnGrabRangeColliderBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex,
	bool				 bFromSweep,
	const FHitResult&	 SweepResult
)
{
	if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor))
		m_GrabRangeEnteredPlayers.Add(Player);
}

void AC_CopZombie::OnGrabRangeColliderEndOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex
)
{
	if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor))
		m_GrabRangeEnteredPlayers.Remove(Player);
}
