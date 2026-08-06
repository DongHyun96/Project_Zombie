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
#include "GameModeAndManager/C_UIManager.h"
#include "Utility/C_Util.h"

AC_ZombieController::AC_ZombieController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 인지기능 컴포넌트 생성 및 Controller에 등록 
	m_PerceptionCom = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*m_PerceptionCom); 

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
	
	m_PerceptionCom->ConfigureSense(*m_SightConfig);
	// 인지 컴포넌트에 시각정보 추가 및 반영 (추후 SightConfig 값을 변경하고 싶다면, 변경한 다음 ConfigureSense 호출을 해주어야 적용된다)
	m_PerceptionCom->ConfigureSense(*m_DamageConfig);

	m_PerceptionCom->SetDominantSense(m_SightConfig->GetSenseImplementation()); // 시각정보를 최우선 감각으로 설정	
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
	m_PerceptionCom->ConfigureSense(*m_SightConfig);
	m_PerceptionCom->ConfigureSense(*m_DamageConfig);
	m_PerceptionCom->RequestStimuliListenerUpdate();

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


void AC_ZombieController::OnTargetDetected(AActor* _Target, FAIStimulus _Stimulus)
{
	if (UAISense::GetSenseID<UAISense_Sight>() == _Stimulus.Type)
	{
		if (_Stimulus.WasSuccessfullySensed())
			UC_Util::Print("DETECTED(Stim -> Sight)" + _Target->GetName(), FColor::Red, 10.f);
		else 
			UC_Util::Print("Get Out (Stim -> Sight)" + _Target->GetName(), FColor::Red, 10.f);
	}
	else
	{
		
		UC_Util::Print("DETECTED(Stim -> Other)" + _Target->GetName(), FColor::Red, 10.f);
	}
	
	
	if (!m_OwnerZombie)
	{
		UC_Util::Print("[AC_ZombieController::OnTargetDetected] : m_OwnerZombie nullptr", FColor::Red, 10.f);
		return;
	}
	
	
	// 감지대상의 우호관계 가져오기
	ETeamAttitude::Type type = m_OwnerZombie->GetTeamAttitudeTowards(*_Target);

	// 감지 대상이 적(플레이어)이라면
	if (ETeamAttitude::Hostile == type)
	{
		// 이미 감지됐던 타겟인지 확인
		FSensedTargetInfo* pInfo = FindSensedTarget(_Target);

		// 없다면 목록에 추가
		if (nullptr == pInfo)
		{
			pInfo = &AddSensedTarget(_Target);
		}

		pInfo->bSensed = _Stimulus.WasSuccessfullySensed();

		// 인지범위에서 벗어난 경우
		if (false == pInfo->bSensed)
		{
			// 놓친 위치 저장
			pInfo->LosePos = _Stimulus.StimulusLocation;

			// 놓쳤을 때 시간 저장
			pInfo->LoseTime = GetWorld()->GetTimeSeconds();
		}

		// 인지정보가 어떤 감각으로 발생한 정보인지 구별
		static FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
		static FAISenseID DmgID = UAISense::GetSenseID<UAISense_Damage>();
		// static FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();

		if (_Stimulus.Type == SightID)
		{
			if (pInfo->bSensed)
				pInfo->AggroValue += 10.f;
		}
		else if (_Stimulus.Type == DmgID)
		{
			PRINT_LOCAL(GetWorld(), "DAMAGE SENSED RECEIVED CALCULATING AGGROVALUE", FColor::Red, 10.f);
			pInfo->AggroValue += 20.f;
		}
		/*else // Not in used (Hearing 자극은 판단 x)
		{
			pInfo->AggroValue += 15.f;
		}*/
	}
}

void AC_ZombieController::BeginPlay()
{
	Super::BeginPlay();
	
	// 탐지가 발생하면 호출받을 Delegate 등록
	m_PerceptionCom->OnTargetPerceptionUpdated.AddDynamic(this, &AC_ZombieController::OnTargetDetected);
}

void AC_ZombieController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	/*TArray<AActor*> Actors;

	m_PerceptionCom->GetKnownPerceivedActors(
		UAISense_Sight::StaticClass(),
		Actors);

	UE_LOG(LogTemp, Warning, TEXT("Known : %d"), Actors.Num());

	for (AActor* Actor : Actors)
	{
		UE_LOG(LogTemp, Warning, TEXT("  %s"), *Actor->GetName());
	}*/
}

FSensedTargetInfo& AC_ZombieController::AddSensedTarget(AActor* _Target)
{
	FSensedTargetInfo info;

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
			// iter 가 가리키는 대상을 삭제하고, 하나 이전을 가리킨다
			iter.RemoveCurrent();
		}
	}
}

AActor* AC_ZombieController::GetCurrentBBTarget() const
{
	return Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TEXT("Target"))) : nullptr;
}

bool AC_ZombieController::IsCurrentlyOnSight(AActor* _TargetActor) const
{
	if (!_TargetActor) return false;
	
	if (const FActorPerceptionInfo* ActorPerceptionInfo = m_PerceptionCom->GetActorInfo(*_TargetActor))
		for (const FAIStimulus& Stimulus : ActorPerceptionInfo->LastSensedStimuli)
		{
			const TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);
			if (SenseClass != UAISense_Sight::StaticClass()) continue; // 다른 인지기관 (Sight 기관이 필요함)
			
			if (Stimulus.WasSuccessfullySensed()) return true;
		}
	
	return false;
}