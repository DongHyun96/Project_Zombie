// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PingSystemComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"


UC_PingSystemComponent::UC_PingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_PingSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AC_BasicPlayer* OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!OwnerPlayer)
	{
		UC_Util::Print("From UC_PingSystemComponent : Wrong OwnerPlayer, this component only owned by player objects", FColor::Red, 10.f);
		return;
	}
	
	if (!m_WorldPingActorClass)
	{
		UC_Util::Print("From UC_PingSystemComponent : Please init pingActorClass subclass on blueprint!", FColor::Red, 10.f);
		return;
	}
	
	FActorSpawnParameters Param{};
	Param.Owner = OwnerPlayer;
	m_WorldPingActor = GetWorld()->SpawnActor<AC_WorldPingActor>(m_WorldPingActorClass, Param); // PingActor BeginPlay에 자기자신 비활성화 처리 들어가 있음
}

bool UC_PingSystemComponent::TrySpawnPing()
{
	APlayerCameraManager* PCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	const FVector CamLocation            = PCameraManager->GetCameraLocation();
	const FVector CamForward             = PCameraManager->GetCameraRotation().Vector();

	// 100m 이내만 찍을 수 있게끔 처리
	const FVector DestLocation = CamLocation + 10000.f * CamForward;

	// TODO : Player Actor들 모두 Ignore 처리 필요
	FCollisionQueryParams CollisionParams{};
	CollisionParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult{};
	
	bool HasHit = GetWorld()->LineTraceSingleByChannel(HitResult, CamLocation, DestLocation, ECC_Visibility, CollisionParams);
	
	if (!HasHit) return false;

	// TODO : 여기서 LineTrace 결과에 따른 PingType 지정해서 제대로 넣어줄 것 (일단은 DefaultMarker로 처리함) 
	m_WorldPingActor->SpawnPingActorToWorld(HitResult);
	return true;
}

void UC_PingSystemComponent::SpawnFullPing(const FVector& _SpawnLocation, EGamePingType _PingType)
{
	m_WorldPingActor->SpawnFullPingActorToWorld(_SpawnLocation, _PingType);
}

