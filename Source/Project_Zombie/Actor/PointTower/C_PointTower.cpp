// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTower.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Components/SphereComponent.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/CompassBarWidget/C_CompassBarWidget.h"

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
	
	m_InteractionTestingCollider = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionTestingCollider"));
	m_InteractionTestingCollider->SetupAttachment(GetRootComponent());
	
	// Agent TeamId (Enemy만 이 Team ID로 적인지 아닌지 구분하기 때문에 Player Team으로 부여시킴)
	m_TeamId = static_cast<uint8>(ETeamType::Player);
}

void AC_PointTower::BeginPlay()
{
	Super::BeginPlay();
	
	// Electronic effect 부모 SceneComponent 초기화
	m_ElectroSplinesParent = FindSceneComponentByName(TEXT("ElectroSplines"));
	if (m_ElectroSplinesParent)
		m_ElectroSplinesParent->SetHiddenInGame(true, true);

	// Damage Detector 초기화
	m_DamageDetector = Cast<UShapeComponent>(FindSceneComponentByName(TEXT("DamageDetector")));
	if (m_DamageDetector)
		m_DamageDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	else UC_Util::Print("Damage Detector init failed!", FColor::Red, 10.f);
	
	if (HasAuthority())
	{
		// 서버 환경에서의 PointTower만 PointTowerManager(서버 쪽에만 존재) 에 등록 처리를 할 것임
		POINT_TOWER_MANAGER(this)->RegisterPointTower(this);
	}

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
	
	m_InteractionTestingCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (HasAuthority()) // 오로지 서버 쪽에서만 이벤트 처리 (간단하게 그냥 함)
	{
		m_InteractionTestingCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_PointTower::OnInteractionColliderBeginOverlap);
		m_InteractionTestingCollider->OnComponentEndOverlap.AddDynamic(this, &AC_PointTower::OnInteractionColliderEndOverlap);
	}
	
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

	if (m_ConqueringPlayer) // Conquering interaction 하는 Player가 존재
	{
		/* 지속적으로 거점 게이지를 활성화한다 */

		m_CurConquerAmount += DeltaTime * m_IncreaseAmountPerSec;
		
		// TODO : 거점을 활성화하는 Player의 Damage 처리는 본인의 Local 환경에서 지속적인 도트데미지 입히기
	}
	else // Conquering interaction 하는 Player가 한 명도 없음
	{
		/* 거점을 점령하는 Player가 없는 경우, 지속적으로 거점 게이지를 떨군다 */
		m_CurConquerAmount -= DeltaTime * m_DefaultDecreasingAmountOfConquerAmountPerSec;
	}
	
	m_CurConquerAmount = FMath::Clamp(m_CurConquerAmount, 0.f, m_MaxConquerAmount);
	
	const uint8 CurrentIntConquerAmount = static_cast<uint8>(m_CurConquerAmount);
	if (CurrentIntConquerAmount != m_CurConquerAmountInt)
	{
		Multicast_UpdateConquerAmountInt(CurrentIntConquerAmount); // 감소된 Int값 동기화
		
		if (CurrentIntConquerAmount >= m_MaxConquerAmount) // 거점 게이지를 모두 채운 상황 
		{
			// 이 거점 Conquered 처리 및 다음 거점 Round 활성화를 해야하는지 체크
			SetPointTowerState(EPointTowerState::Conquered);
			POINT_TOWER_MANAGER(this)->OnPointTowerConquered();
		}
	}
}

ETeamAttitude::Type AC_PointTower::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
	
	if (!OtherTeamAgent) return ETeamAttitude::Neutral;
	return m_TeamId == OtherTeamAgent->GetGenericTeamId() ? ETeamAttitude::Friendly : ETeamAttitude::Hostile;	
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
		m_DamageDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EPointTowerState::Active:
	{
		Multicast_Activate(); // 방장을 포함한 모든 사람들 Activate 상태로 변경
		m_InteractionTestingCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 서버 쪽 환경에서만 Collision On 처리됨
		m_DamageDetector->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		break;
	}
	case EPointTowerState::Conquered:
		m_DamageDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Multicast_Conquered();
		m_InteractionTestingCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_ConquerTestAreaEnteredPlayers.Empty();
		m_ConqueringPlayer = nullptr; // 마지막으로 거점 활성화 처리를 했었던 Player의 도트데미지 제거하는 알림을 Client_~ 뭐시기로 쏠 것
	}
}

bool AC_PointTower::CanCurrentlyAttackedByZombie()
{
	if (m_State == EPointTowerState::Waiting) return false;
	
	// 현재 활성화된 시퀀스의 PointTower일 경우에만 공격 가능
	return POINT_TOWER_MANAGER(this)->GetCurrentSequenceIdx() == m_ActivateSequenceIdx;  
}

bool AC_PointTower::CanBeInsertedToSensedTarget()
{
	UC_PointTowerManager* PointTowerManager = POINT_TOWER_MANAGER(this);
	if (!PointTowerManager) return false; // 아직 게임 시작도 전에 물어본 상황 (또는 Client 환경에서 물어본 상황)	

	// 이미 이전 라운드에 해당되는 PointTower인 경우
	if (m_ActivateSequenceIdx < PointTowerManager->GetCurrentSequenceIdx()) return false;

	// 이번 라운드 또는, 다음 라운드에 활성화될 거점인 경우 -> SensedInfo로 들어갈 수 있는 상황
	return true;
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

void AC_PointTower::OnInteractionColliderBeginOverlap
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
	if (!EnteredPlayer) return;

	if (!m_ConqueringPlayer) m_ConqueringPlayer = EnteredPlayer; // 가장 먼저 들어온 Player의 경우, 해당 Player를 ConqueringPlayer로 등록 -> TODO : 이 친구 도트데미지 처리해 줄 것
	
	// 이 Area안에 들어온 모든 Player를 일단 저장해둠 (추후 후보군을 두기 위함)
	m_ConquerTestAreaEnteredPlayers.Add(EnteredPlayer);
}

void AC_PointTower::OnInteractionColliderEndOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex
)
{
	// 활성화 상태가 아님
	if (m_State != EPointTowerState::Active) return;
	
	AC_BasicPlayer* ExitPlayer = Cast<AC_BasicPlayer>(OtherActor);
	if (!ExitPlayer) return;

	m_ConquerTestAreaEnteredPlayers.Remove(ExitPlayer);

	// 거점 활성화 대상자가 나간 건 아닌 경우
	if (ExitPlayer != m_ConqueringPlayer)  return;
	
	// 방금 전까지 거점 활성화 대상자가 나간 상황
	m_ConqueringPlayer = nullptr; // TODO : 이전 대상자 도트데미지 제거해주어야 함
	// -> TODO : Local Player에게 거점 활성화 중이 아니라고 알려주어야 함 (도트데미지 적용 효과 제거)
	// Client_~~~ 로 알려주기
	
	// 아직 나가지 않은 거점 점령 대상 후보군 중 가장 가까운 후보군으로 다음 거점 점령자 등록
	float MinDistSqr = FLT_MAX;
	AC_BasicPlayer* PickedPlayer{};
	const FVector InteractionTestingColliderLocation = m_InteractionTestingCollider->GetComponentLocation();
	for (AC_BasicPlayer* EnteredPlayer : m_ConquerTestAreaEnteredPlayers)
	{
		const float DistSqr = FVector::DistSquared(EnteredPlayer->GetActorLocation(), InteractionTestingColliderLocation);
		if (DistSqr < MinDistSqr)
		{
			MinDistSqr = DistSqr;
			PickedPlayer = EnteredPlayer;
		}
	}
	
	// 가장 가까운 플레이어를 다음 점령자로 등록
	m_ConqueringPlayer = PickedPlayer;
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

float AC_PointTower::TakeDamage
(
	float				DamageAmount,
	FDamageEvent const& DamageEvent,
	AController*		EventInstigator,
	AActor*				DamageCauser
)
{
	// 현재 데미지를 입을 수 없는 상황인데 공격을 당한 경우
	if (!CanCurrentlyAttackedByZombie()) return 0.f;
	
	float ReceivedDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float Damage = ReceivedDamageAmount * m_ZombieDamageRatio; // Damage 만큼 현재 Conquered 펀센트에서 제거
	m_CurConquerAmount -= Damage;
	return Damage;
}

void AC_PointTower::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	// 예: 메시의 'head' 소켓 위치를 AI 감지 포인트로 지정
	/*if (m_StaticMeshComTower->DoesSocketExist(TEXT("SightViewSocket")))
	{
		OutLocation = m_StaticMeshComTower->GetSocketLocation(TEXT("SightViewSocket"));
		OutRotation = m_StaticMeshComTower->GetSocketRotation(TEXT("SightViewSocket"));
	}*/

	// 소켓이 없는 경우 기본 C++ 구현(Actor Location) 호출
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);
}

void AC_PointTower::Multicast_Activate_Implementation()
{
	if (!HasActorBegunPlay())
	{
		PRINT_LOCAL(GetWorld(), "[AC_PointTower::Multicast_Activate] : PointTower is not yet BeginPlayed.", FColor::Red, 10.f);
		return;
	}

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
	if (!HasActorBegunPlay())
	{
		PRINT_LOCAL(GetWorld(), "[AC_PointTower::Multicast_Conquered] : PointTower is not yet BeginPlayed.", FColor::Red, 10.f);
		return;
	}

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