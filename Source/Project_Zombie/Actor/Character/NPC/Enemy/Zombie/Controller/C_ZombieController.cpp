// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieController.h"

// PerceptionComponent
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"

// TeamAgentInterface
#include "GenericTeamAgentInterface.h"

// BehaviorTree / Blackboard
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

// StatComponent

#include "../../../../../../GlobalEnum.h"

#include "../C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Components/StatComponent/C_EnemyStatComponent.h"
#include "Actor/PointTower/C_PointTower.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"
#include "WorldPartition/HLOD/HLODRuntimeSubsystem.h"

AC_ZombieController::AC_ZombieController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// 인지기능 컴포넌트 생성 및 Controller에 등록 
	m_PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SetPerceptionComponent(*m_PerceptionComponent); 

	// 시각정보 설정
	m_SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));

	if (m_SightConfig)
	{
		// 시야 기본값 세팅
		m_SightConfig->SightRadius                              = 3000.f; // AI 가 대상을 처음 감지할 수 있는 거리
		m_SightConfig->LoseSightRadius                          = 3500.f; // AI 가 대상을 처음 감지할 수 있는 거리
		m_SightConfig->PeripheralVisionAngleDegrees             = 60.f; // 시전 정면방향을 기준으로, 반경 각도, 최대시야각은 x2 
		m_SightConfig->DetectionByAffiliation.bDetectEnemies    = true; // 감지대상이 적대관계인경우 탐지한것으로 인정
		m_SightConfig->DetectionByAffiliation.bDetectFriendlies = false; // 감지대상이 우호관계인경우 탐지 x
		m_SightConfig->DetectionByAffiliation.bDetectNeutrals   = false; // 감지대상이 중립관계인경우 탐지 x
	}
	m_DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage"));
	
	m_PerceptionComponent->ConfigureSense(*m_SightConfig);
	// 인지 컴포넌트에 시각정보 추가 및 반영 (추후 SightConfig 값을 변경하고 싶다면, 변경한 다음 ConfigureSense 호출을 해주어야 적용된다)
	m_PerceptionComponent->ConfigureSense(*m_DamageConfig);

	m_PerceptionComponent->SetDominantSense(m_SightConfig->GetSenseImplementation()); // 시각정보를 최우선 감각으로 설정	
}

void AC_ZombieController::OnPossess(APawn* _Pawn)
{
	Super::OnPossess(_Pawn);

	m_OwnerZombie = Cast<AC_Zombie>(GetPawn());

	//UE_LOG(LogTemp, Warning, TEXT("OnPossess Success"));

	// 빙의한 대상과 같은 팀으로 설정
	const IGenericTeamAgentInterface* pPawnTeam = Cast<IGenericTeamAgentInterface>(_Pawn);
	
	if (pPawnTeam) SetGenericTeamId(pPawnTeam->GetGenericTeamId());
	else SetGenericTeamId(static_cast<uint8>(ETeamType::None));

	// 빙의 대상의 스탯 컴포넌트를 가져온다
	UC_EnemyStatComponent* pStatCom = _Pawn->FindComponentByClass<UC_EnemyStatComponent>();

	// 시야 인지범위를 데이터테이블에 작성된 스탯으로 설정
	m_SightConfig->SightRadius = pStatCom->GetStat(TEXT("DetectRange"));
	m_SightConfig->LoseSightRadius = pStatCom->GetStat(TEXT("LoseDetectRange"));

	// 인지 컴포넌트 갱신
	m_PerceptionComponent->ConfigureSense(*m_SightConfig);
	m_PerceptionComponent->ConfigureSense(*m_DamageConfig);
	m_PerceptionComponent->RequestStimuliListenerUpdate();

	// 비헤이비어트리, 블랙보드 세팅
	if (m_BTAsset && m_BBAsset)
	{
		UBlackboardComponent* pBBCom = Blackboard;

		// 블랙보드 에셋을 전달하면, 그걸 사용할 컴포넌트도 만들고, 전달한 블랙보드 에셋을 사용하도록 세팅한다.
		if (UseBlackboard(m_BBAsset, pBBCom))
		{
			// AIController 에게 생성된 블랙보드 컴포넌트 주소를 알려준다. 
			Blackboard = pBBCom;

			RunBehaviorTree(m_BTAsset);
		}
	}
}


void AC_ZombieController::OnTargetUpdated(AActor* _Target, FAIStimulus _Stimulus)
{
	// 자극의 근원지를 추적할 수 없음
	if (!_Target) return;
	
	if (!m_OwnerZombie)
	{
		UC_Util::Print("[AC_ZombieController::OnTargetDetected] : m_OwnerZombie nullptr", FColor::Red, 10.f);
		return;
	}

	// 감지대상의 우호관계 가져오기
	ETeamAttitude::Type type = m_OwnerZombie->GetTeamAttitudeTowards(*_Target);

	// 감지 대상이 적대관계인 경우만 처리
	if (type != ETeamAttitude::Hostile) return;
	
	// PointTower인 경우, 현재 SensedInfo로 넣을 수 있는 상태인지 체크 -> 빼버리는건 Delegate 호출에 의해 알아서 처리가 됨
	AC_PointTower* PointTower = Cast<AC_PointTower>(_Target);
	if (PointTower && !PointTower->CanBeInsertedToSensedTarget()) return;
	
	// 이미 감지됐던 타겟인지 확인
	FSensedTargetInfo* pInfo = FindSensedTarget(_Target);
	if (!pInfo)
	{
		pInfo = &AddSensedTarget(_Target); // 없다면 목록에 추가
		if (PointTower)
		{
			PointTower->m_OnCurPointTowerSequenceOver.AddUObject
			(
				this,
				&AC_ZombieController::OnCurPointSeqOver,
				pInfo // Payload -> 구독을 걸어둘 Info 정보를 Payload로 Delegate 구독자에게 알려놓음
			);
		}
	}

	// 감지 여부를 기록한다
	pInfo->bSensed = _Stimulus.WasSuccessfullySensed();

	/* 인지범위에서 벗어난 경우 */
	if (!pInfo->bSensed)
	{
		UC_Util::Print("OUT", FColor::Red, 10.f);
		pInfo->LosePos  = _Stimulus.StimulusLocation;	// 놓친 위치 저장
		pInfo->LoseTime = GetWorld()->GetTimeSeconds(); // 놓쳤을 때 시간 저장
		return;
	}

	/* 인지범위에 잡힌 경우 */
	UC_Util::Print("IN", FColor::Red, 10.f);
	// 인지정보가 어떤 감각으로 발생한 정보인지 구별
	static FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
	static FAISenseID DmgID   = UAISense::GetSenseID<UAISense_Damage>();
	// static FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();

	if (_Stimulus.Type == SightID)
	{
		if (pInfo->bSensed) pInfo->AggroValue += 10.f; // 이 Target에 대한 어그로수치 10점 추가
	}
	else if (_Stimulus.Type == DmgID) pInfo->AggroValue += 20.f;
}

void AC_ZombieController::BeginPlay()
{
	Super::BeginPlay();
	
	// 탐지가 발생하면 호출받을 Delegate 등록
	m_PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AC_ZombieController::OnTargetUpdated);
}

void AC_ZombieController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// DrawDebugSightRange();
	
	/*TArray<AActor*> Actors;

	m_PerceptionCom->GetKnownPerceivedActors(
		UAISense_Sight::StaticClass(),
		Actors);

	UE_LOG(LogTemp, Warning, TEXT("Known : %d"), Actors.Num());

	for (AActor* Actor : Actors)
	{
		UE_LOG(LogTemp, Warning, TEXT("  %s"), *Actor->GetName());
	}*/
	/*TArray<UObject*> Temp = m_PerceptionComponent->OnTargetPerceptionUpdated.GetAllObjects();
	UC_Util::Print(Temp.Num(), FColor::Red, 1.f);*/
}

FSensedTargetInfo& AC_ZombieController::AddSensedTarget(AActor* _Target)
{
	FSensedTargetInfo info{};

	info.Target = _Target;

	return m_SensedTargets.Add_GetRef(info);
}

FSensedTargetInfo* AC_ZombieController::FindSensedTarget(const AActor* _Target)
{
	for (FSensedTargetInfo& info : m_SensedTargets)
	{
		if (info.Target == _Target)
		{
			return &info;
		}
	}

	return nullptr;
}

void AC_ZombieController::ClearSensedTarget(float _LimitTime)
{
	// 월드 현재시간
	float CurTime = GetWorld()->GetTimeSeconds();

	for (auto iter = m_SensedTargets.CreateIterator(); iter; ++iter)
	{
		bool bRemove = false;

		// 1. 감지한 대상이 삭제된 경우
		if (false == iter->Target.IsValid())
		{
			bRemove = true;
		}

		// 2. 인지를 놓친지 _LimitTime 을 넘어선 경우
		else if (false == iter->bSensed)
		{
			// 감지를 못한 시간이 Limit 를 넘어서면
			if (_LimitTime < CurTime - iter->LoseTime)
			{
				bRemove = true;
			}
		}

		if (bRemove)
		{
			// 제거대상이 PointTower인 경우, Delegate 구독 취소
			if (AC_PointTower* PointTower = Cast<AC_PointTower>(iter->Target))
				PointTower->m_OnCurPointTowerSequenceOver.RemoveAll(this);
			
			// iter 가 가리키는 대상을 삭제하고, 하나 이전을 가리킨다
			iter.RemoveCurrent();
		}
	}
}

void AC_ZombieController::ClearAllSensedTarget()
{
	// 모든 SensedTarget을 지우기 이전 PointTower Target의 경우, Delegate 구독 해지 처리를 해준다
	for (const FSensedTargetInfo& _Info : m_SensedTargets)
	{
		if (AC_PointTower* PointTower = Cast<AC_PointTower>( _Info.Target))
			PointTower->m_OnCurPointTowerSequenceOver.RemoveAll(this);
	}
	
	m_SensedTargets.Empty();
}

AActor* AC_ZombieController::GetCurrentBBTarget() const
{
	return Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TEXT("Target"))) : nullptr;
}

bool AC_ZombieController::IsCurrentlyOnSight(AActor* _TargetActor) const
{
	if (!_TargetActor) return false;
	
	if (const FActorPerceptionInfo* ActorPerceptionInfo = m_PerceptionComponent->GetActorInfo(*_TargetActor))
		for (const FAIStimulus& Stimulus : ActorPerceptionInfo->LastSensedStimuli)
		{
			const TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);
			if (SenseClass != UAISense_Sight::StaticClass()) continue; // 다른 인지기관 (Sight 기관이 필요함)
			
			if (Stimulus.WasSuccessfullySensed()) return true;
		}
	
	return false;
}

void AC_ZombieController::OnCurPointSeqOver(FSensedTargetInfo* _TargetInfo)
{
	if (!_TargetInfo) return;

	const int32 Index = _TargetInfo - m_SensedTargets.GetData();

	if (m_SensedTargets.IsValidIndex(Index))
		m_SensedTargets.RemoveAt(Index);
}

void AC_ZombieController::DrawDebugSightRange()
{
	FVector Center = GetPawn()->GetActorLocation();

	DrawDebugCircle
	(
		GetWorld(),
		Center,
		m_SightConfig->SightRadius,
		300,
		FColor::Green,
		false,
		-1.f, 0, 0,
		GetPawn()->GetActorRightVector(),
		GetPawn()->GetActorForwardVector()
	);

	/*Center.Z += 1.f;

	DrawDebugCircle
	(
		GetWorld(),
		Center,
		BehaviorRange,
		300,
		FColor::Red,
		false,
		-1.f, 0, 0,
		GetPawn()->GetActorRightVector(),
		GetPawn()->GetActorForwardVector()
	);*/
}
