// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTower.h"

#include "ToolBuilderUtil.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Components/SphereComponent.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/CompassBarWidget/C_CompassBarWidget.h"
#include "UI/MainHUD/CompassBarWidget/CompassMarkerWidget/C_CompassMarkerWidget.h"
#include "UI/Misc/C_PointTowerWidget.h"
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
	
	// Electronic effect 부모 SceneComponent 초기화
	m_ElectroSplinesParent = FindSceneComponentByName(TEXT("ElectroSplines"));
	if (m_ElectroSplinesParent)
		m_ElectroSplinesParent->SetHiddenInGame(true, true);

	// 서버 환경에서의 PointTower만 PointTowerManager(서버 쪽에만 존재) 에 등록 처리를 할 것임
	if (HasAuthority())
		POINT_TOWER_MANAGER(this)->RegisterPointTower(this);

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
	/*if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			m_TestTimerHandle, this, &AC_PointTower::TestFunction, 10.f, false
		);
		
		GetWorld()->GetTimerManager().SetTimer
		(
			m_TestTimerHandle2, this, &AC_PointTower::TestFunction2, 20.f, false
		);
	}*/
	// Activate(0.f);
}

void AC_PointTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 매 Tick 업데이트될 거점 게이지 정확한 퍼센티지 등은 서버 쪽에서만 처리
	if (!HasAuthority()) return;
	
	if (m_State != EPointTowerState::Active) return;

	/* 현재 거점이 열린 상태 */

	if (m_ConqueringPlayer) return; // 점령을 하는 중인 Player가 있을 때 -> Damage 처리는 각자의 Local에서 실시간으로 처리할 것
	
	/* 거점을 점령하는 Player가 없는 경우, 지속적으로 거점 게이지를 떨군다 */
	m_CurConquerAmount -= DeltaTime * m_DefaultDecreasingAmountOfConquerAmountPerSec;	
	m_CurConquerAmount = FMath::Max(0.f, m_CurConquerAmount);
	
	const uint8 CurrentIntConquerAmount = static_cast<uint8>(m_CurConquerAmount);
	if (CurrentIntConquerAmount != m_CurConquerAmountInt)
	{
		Multicast_UpdateConquerAmountInt(CurrentIntConquerAmount); // 감소된 Int값 동기화
		// if (CurrentIntConquerAmount == m_MaxConquerAmount)
	}
}

void AC_PointTower::SetPointTowerState(EPointTowerState _PointTowerState)
{
	// Waiting -> Active -> Conquered && Conquered -> Active (인 Transition 처리 밖에 없긴한데, 일단은 외부에서 sequence대로 잘 호출처리되는 걸로 생각하고 진행함)
	
	// 오로지 서버 쪽에서만 PointTower의 상태전환 처리를 할 예정
	if (!HasAuthority()) return;
	
	// 이미 해당 상황인 경우
	if (m_State == _PointTowerState) return;
	
	// PointTowerState 갱신
	m_State = _PointTowerState;
	
	switch (_PointTowerState)
	{
	case EPointTowerState::Waiting: // 당장에는 Nothing to do
		break;
	case EPointTowerState::Active:
	{
		Multicast_Activate(); // 방장을 포함한 모든 사람들 Activate 상태로 변경
		break;
	}
	case EPointTowerState::Conquered:
		Multicast_Conquered();
	}
}

void AC_PointTower::TestFunction()
{
	SetPointTowerState(EPointTowerState::Active);
}

void AC_PointTower::TestFunction2()
{
	SetPointTowerState(EPointTowerState::Conquered);
}

void AC_PointTower::Multicast_UpdateConquerAmountInt_Implementation(uint8 _CurrentConquerAmount)
{
	m_CurConquerAmountInt = _CurrentConquerAmount;

	// 점령 퍼센트 UI 업데이트
	if (m_PointTowerWidget) m_PointTowerWidget->SetPercentText(m_CurConquerAmountInt);
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
	if (m_State != EPointTowerState::Active) return;
	
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
	if (m_State != EPointTowerState::Active) return;
	
	AC_BasicPlayer* EnteredPlayer = Cast<AC_BasicPlayer>(OtherActor);
	if (!EnteredPlayer || !EnteredPlayer->IsLocallyControlled()) return; // 들어온 Actor가 Player가 아니거나, 내가 조종하는 Player가 아님
	
	// MainMesh의 Outline 재활성화
	m_StaticMeshComTower->SetCustomDepthStencilValue(2);
}

USceneComponent* AC_PointTower::FindSceneComponentByName(const FName& _ComName)
{
	TArray<UActorComponent*> Components{};
	GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (Component && Component->GetFName() == _ComName)
			return Cast<USceneComponent>(Component);
	}

	return nullptr;
}

void AC_PointTower::Multicast_Activate_Implementation()
{
	if (!HasActorBegunPlay()) return;

	m_State = EPointTowerState::Active;
	
	// 거점 활성화 Outline 활성화
	m_StaticMeshComTower->SetCustomDepthStencilValue(2);
	m_StaticMeshComGenerator->SetCustomDepthStencilValue(2);
	
	// 핑 활성화
	if (m_WorldPingActor)
	{
		const FVector GeneratorLocation = m_StaticMeshComGenerator->GetComponentLocation();
		
		m_WorldPingActor->SpawnPingActorToWorld(GeneratorLocation, EGamePingType::AntennaMarker, EPingShapeType::FullPing);
		
		m_ActivatedCompassMarkerWidget = UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetCompassBarWidget()->SpawnGlobalPingMarker
		(
			EGamePingType::AntennaMarker,
			GeneratorLocation
		);		
	}
	
	// 근접 접근 시, EffectToggling 처리용 감지 Collider 활성화
	m_ApproachEffectTogglerCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 점령 게이지 보여주는 Widget 활성화
	if (m_PointTowerWidget)
	{
		m_PointTowerWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		m_PointTowerWidget->SetPercentText(m_CurConquerAmountInt);
	}
}

void AC_PointTower::Multicast_Conquered_Implementation()
{
	if (!HasActorBegunPlay()) return;

	m_State = EPointTowerState::Conquered;

	// 거점 아웃라인 비활성화
	m_StaticMeshComTower->SetCustomDepthStencilValue(0);
	m_StaticMeshComGenerator->SetCustomDepthStencilValue(0);
	
	// 핑 비활성화
	if (m_WorldPingActor)
	{
		m_WorldPingActor->HidePing();
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetCompassBarWidget()->HideGlobalPingMarker(m_ActivatedCompassMarkerWidget);
	}
	
	// 근접 접근 시, EffectToggling 처리용 감지 Collider 비활성화
	m_ApproachEffectTogglerCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 전기 Effect 보여주기
	if (m_ElectroSplinesParent)
		m_ElectroSplinesParent->SetHiddenInGame(false, true);
	
	if (m_PointTowerWidget) m_PointTowerWidget->SetVisibility(ESlateVisibility::Collapsed);
}