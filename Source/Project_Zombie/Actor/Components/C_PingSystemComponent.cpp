// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PingSystemComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Ping/C_PlayerWorldPingActor.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerProfileComponent/C_PlayerProfileComponent.h"
#include "Utility/C_Util.h"


UC_PingSystemComponent::UC_PingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}

void UC_PingSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
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
	Param.Owner      = m_OwnerPlayer;
	UC_Util::Print("Spawning WorldPingActor", FColor::Red, 15.f);
	m_WorldPingActor = GetWorld()->SpawnActor<AC_PlayerWorldPingActor>(m_WorldPingActorClass, Param);
	// PingActor BeginPlay에 자기자신 비활성화 처리 들어가 있음
}

bool UC_PingSystemComponent::TrySpawnPing(UObject* _Instigator)
{
	// 0. LOCAL 환경
	// 1. 카메라가 바라보는 방면으로 RayTrace
	// 2. Hit한 지점에 Ping 스폰 처리 vs Hit 하지 않았다면 스폰시키지 않음
	if (!m_OwnerPlayer->IsLocallyControlled()) return false;
	
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
	const EGamePingType GamePingType = EGamePingType::DefaultMarker;
	// 대략 45도까지는 바닥면으로 간주
	const EPingShapeType PingShapeType = (HitResult.ImpactNormal.Z < 0.7f) ? EPingShapeType::IconPing : EPingShapeType::FullPing;
	m_WorldPingActor->SpawnPingActorToWorld(HitResult.ImpactPoint, GamePingType, PingShapeType);
	
	m_WorldPingActor->SetPingColor(m_OwnerPlayer->GetPlayerProfileComponent()->GetPlayerSelectedColor());
	
	// 서버로 Ping Spawn 처리되었다고 알림
	Server_SpawnPing(HitResult.ImpactPoint, GamePingType, PingShapeType);

	// 마지막 PingSpawn Instigator로 등록
	m_LastInstigator = _Instigator;
	
	return true;
}

void UC_PingSystemComponent::SpawnFullPing(const FVector& _SpawnLocation, EGamePingType _PingType, UObject* _Instigator)
{
	m_WorldPingActor->SpawnPingActorToWorld(_SpawnLocation, _PingType, EPingShapeType::FullPing);
	m_WorldPingActor->SetPingColor(m_OwnerPlayer->GetPlayerProfileComponent()->GetPlayerSelectedColor());
	Server_SpawnPing(_SpawnLocation, _PingType, EPingShapeType::FullPing);
	m_LastInstigator = _Instigator;
}

void UC_PingSystemComponent::HidePing()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;
	
	m_WorldPingActor->HidePing();
	m_LastInstigator = nullptr;
}

void UC_PingSystemComponent::Multicast_MustHidePingAll_Implementation()
{
	m_WorldPingActor->HidePing();
	m_LastInstigator = nullptr;
}

void UC_PingSystemComponent::Server_SpawnPing_Implementation(const FVector& _SpawnedLocation, EGamePingType _GamePingType, EPingShapeType _PingShapeType)
{
	Multicast_SpawnPing
	(
		_SpawnedLocation,
		_GamePingType,
		_PingShapeType
	);
}

bool UC_PingSystemComponent::Server_SpawnPing_Validate(const FVector& _SpawnedLocation, EGamePingType _GamePingType, EPingShapeType _PingShapeType)
{
	return true;
}

void UC_PingSystemComponent::Multicast_SpawnPing_Implementation
(
	const FVector&	_SpawnedLocation,
	EGamePingType	_GamePingType,
	EPingShapeType	_PingShapeType
)
{
	// 자기자신의 Ping Spawn은 바로 Local 환경에서 이미 띄운 상황

	// 아직 BeginPlay 호출 이전인 경우, 남의 PingSpawn 자체를 넘어감
	if (!HasBegunPlay()) return;
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	
	m_WorldPingActor->SetPingColor(m_OwnerPlayer->GetPlayerProfileComponent()->GetPlayerSelectedColor());
	m_WorldPingActor->SpawnPingActorToWorld(_SpawnedLocation, _GamePingType, _PingShapeType);
}

void UC_PingSystemComponent::Multicast_MustSpawnAll_Implementation
(
	const FVector&	_SpawnedLocation,
	EGamePingType	_GamePingType,
	EPingShapeType	_PingShapeType,
	AActor*			_LastInstigator
)
{
	// 아직 BeginPlay 호출 이전인 경우, 남의 PingSpawn 자체를 넘어감
	if (!HasBegunPlay()) return;
	
	// 무조건 스폰 처리
	
	m_WorldPingActor->SetPingColor(m_OwnerPlayer->GetPlayerProfileComponent()->GetPlayerSelectedColor());
	m_WorldPingActor->SpawnPingActorToWorld(_SpawnedLocation, _GamePingType, _PingShapeType);
	m_LastInstigator = _LastInstigator;
}

void UC_PingSystemComponent::Server_HidePing_Implementation()
{
	Multicast_HidePing();
}

bool UC_PingSystemComponent::Server_HidePing_Validate()
{
	return true;
}

void UC_PingSystemComponent::Multicast_HidePing_Implementation()
{
	if (!HasBegunPlay()) return;
	
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	m_WorldPingActor->HidePing();
}



/*void UC_PingSystemComponent::SetPingColor(const FColor& _PingColor)
{
	if (!m_WorldPingActor)
	{
		PRINT_LOCAL(GetWorld(), "From UC_PingSystemComponent::SetPingColor : WorldPingActor not spawned yet", FColor::Red, 10.f);
		return;
	}
	
	m_WorldPingActor->SetPingColor(_PingColor);
}*/
