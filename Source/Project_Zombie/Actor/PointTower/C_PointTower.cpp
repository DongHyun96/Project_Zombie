// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTower.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Components/SphereComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/CompassBarWidget/C_CompassBarWidget.h"
#include "Utility/C_Util.h"

const float AC_PointTower::m_MaxConquerAmount = 100.f;

AC_PointTower::AC_PointTower()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetReplicates(true);

	m_StaticMeshComGenerator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComGenerator"));
	m_StaticMeshComTower	 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComTower"));
	
	SetRootComponent(m_StaticMeshComTower);
	m_StaticMeshComGenerator->SetupAttachment(GetRootComponent());

	// Outline 기능에 필요한 CustomDepth 활성화
	m_StaticMeshComGenerator->SetRenderCustomDepth(true);
	m_StaticMeshComTower->SetRenderCustomDepth(true);
	
	m_ApproachEffectTogglerCollider = CreateDefaultSubobject<USphereComponent>(TEXT("ApproachEffectTogglerCollider"));
	m_ApproachEffectTogglerCollider->SetupAttachment(GetRootComponent());

	m_StaticMeshComTower->SetComponentTickEnabled(false);
	m_StaticMeshComGenerator->SetComponentTickEnabled(false);
	m_ApproachEffectTogglerCollider->SetComponentTickEnabled(false);
	
}

void AC_PointTower::BeginPlay()
{
	Super::BeginPlay();

	// 서버 환경에서의 PointTower만 PointTowerManager(서버 쪽에만 존재) 에 등록 처리를 할 것임
	/*if (HasAuthority())
		POINT_TOWER_MANAGER(this)->RegisterPointTower(this);*/		

	if (m_WorldPingActorClass)
	{
		FActorSpawnParameters Param{};
		Param.Owner      = this;
		m_WorldPingActor = GetWorld()->SpawnActor<AC_WorldPingActor>(m_WorldPingActorClass, Param);
	}
	else UC_Util::Print("[AC_PointTower::BeginPlay] : Please init worldPingActorClass", FColor::Red, 10.f);
	

	m_ApproachEffectTogglerCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_ApproachEffectTogglerCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_PointTower::OnApproachEffectTogglerColliderBeginOverlap);
	m_ApproachEffectTogglerCollider->OnComponentEndOverlap.AddDynamic(this, &AC_PointTower::OnApproachEffectTogglerColliderEndOverlap);
	
	// For Testing
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			m_TestTimerHandle, this, &AC_PointTower::TestFunction, 10.f, false
		);
	}
	// Activate(0.f);
}

void AC_PointTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_PointTower::SetPointTowerState(EPointTowerState _PointTowerState)
{
	// 이미 해당 상황인 경우
	if (m_State == _PointTowerState) return;
	
	// Waiting -> Active -> Conquered && Conquered -> Active (인 Transition 처리 밖에 없긴한데, 일단은 외부에서 sequence대로 잘 호출처리되는 걸로 생각하고 진행함)

	m_State = _PointTowerState;
	
	switch (_PointTowerState)
	{
	case EPointTowerState::Waiting: // 당장에는 Nothing to do
		break;
	case EPointTowerState::Active:
		break;
	case EPointTowerState::Conquered:
		break;
	}
	
}

bool AC_PointTower::Activate(float _ActivatedConquerAmount)
{
	if (!HasAuthority())
	{
		UC_Util::Print("[AC_PointTower::Activate] : Only call Activate function in server!", FColor::Red, 10.f);
		return false;
	}

	m_bIsActive = true;
	m_CurConquerAmount = _ActivatedConquerAmount;
	Multicast_Activate();
	
	return true;
}

void AC_PointTower::TestFunction()
{
	Activate(0.f);
}

void AC_PointTower::OnApproachEffectTogglerColliderBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex,
	bool			  	 bFromSweep,
	const FHitResult& 	 SweepResult
)
{
	// 활성화 상태가 아님
	if (!m_bIsActive) return;
	
	AC_BasicPlayer* EnteredPlayer = Cast<AC_BasicPlayer>(OtherActor);
	if (!EnteredPlayer || !EnteredPlayer->IsLocallyControlled()) return; // 들어온 Actor가 Player가 아니거나, 내가 조종하는 Player가 아님
	
	// MainMesh의 Outline 비활성화
	m_StaticMeshComTower->SetCustomDepthStencilValue(0);
}

void AC_PointTower::OnApproachEffectTogglerColliderEndOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex
)
{
	// 활성화 상태가 아님
	if (!m_bIsActive) return;
	
	AC_BasicPlayer* EnteredPlayer = Cast<AC_BasicPlayer>(OtherActor);
	if (!EnteredPlayer || !EnteredPlayer->IsLocallyControlled()) return; // 들어온 Actor가 Player가 아니거나, 내가 조종하는 Player가 아님
	
	// MainMesh의 Outline 재활성화
	m_StaticMeshComTower->SetCustomDepthStencilValue(2);
}

void AC_PointTower::Multicast_Activate_Implementation()
{
	// if (!HasActorBegunPlay()) return;

	PRINT_LOCAL(GetWorld(), "MULTICAST", FColor::Red, 10.f);
	
	m_bIsActive = true;
	
	// 거점 활성화 Outline 활성화
	m_StaticMeshComTower->SetCustomDepthStencilValue(2);
	m_StaticMeshComGenerator->SetCustomDepthStencilValue(2);
	
	// 핑 활성화
	if (m_WorldPingActor)
	{
		const FVector GeneratorLocation = m_StaticMeshComGenerator->GetComponentLocation();
		
		m_WorldPingActor->SpawnPingActorToWorld(GeneratorLocation, EGamePingType::AntennaMarker, EPingShapeType::FullPing);
		
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetCompassBarWidget()->SpawnGlobalPingMarker
		(
			EGamePingType::AntennaMarker,
			GeneratorLocation
		);		
	}
	
	// 근접 접근 시, EffectToggling 처리용 감지 Collider 활성화
	m_ApproachEffectTogglerCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

