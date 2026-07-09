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
#include "../../C_EnemyStatComponent.h"

#include "../../../../../../GlobalEnum.h"

#include "../C_Zombie.h"

AC_ZombieController::AC_ZombieController()
{
	// 인지기능 컴포넌트 생성 및 Controller에 등록 
	m_PerceptionCom = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*m_PerceptionCom); 

	// 시각정보 설정
	m_SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));

	m_HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing"));

	if (m_SightConfig)
	{
		// 시야 기본값 세팅
		m_SightConfig->SightRadius = 3000.f; // AI 가 대상을 처음 감지할 수 있는 거리
		m_SightConfig->LoseSightRadius = 3500.f; // AI 가 대상을 처음 감지할 수 있는 거리
		m_SightConfig->PeripheralVisionAngleDegrees = 60.f; // 시전 정면방향을 기준으로, 반경 각도, 최대시야각은 x2 
		m_SightConfig->DetectionByAffiliation.bDetectEnemies = true; // 감지대상이 적대관계인경우 탐지한것으로 인정
		m_SightConfig->DetectionByAffiliation.bDetectFriendlies = true; // 감지대상이 우호관계인경우 탐지한것으로 인정
		m_SightConfig->DetectionByAffiliation.bDetectNeutrals = true; // 감지대상이 중립관계인경우 탐지한것으로 인정

		m_PerceptionCom->ConfigureSense(*m_SightConfig); // 인지 컴포넌트에 시각정보 추가
		m_PerceptionCom->SetDominantSense(m_SightConfig->GetSenseImplementation()); // 시각정보를 최우선 감각으로 사용할 것
	}

	if (m_HearingConfig)
	{
		// 청각 기본값 세팅
		m_HearingConfig->HearingRange = 2000.f;

		m_HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		m_HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
		m_HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;

		m_PerceptionCom->ConfigureSense(*m_HearingConfig); // 인지 컴포넌트에 청각정보 추가
	}

	m_DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage"));
	m_PerceptionCom->ConfigureSense(*m_DamageConfig); // 인지 컴포넌트에 데미지정보 추가


}

void AC_ZombieController::OnPossess(APawn* _Pawn)
{
	Super::OnPossess(_Pawn);

	UE_LOG(LogTemp, Warning, TEXT("OnPossess Success"));

	// 빙의한 대상과 같은 팀으로 설정
	const IGenericTeamAgentInterface* pPawnTeam = Cast<IGenericTeamAgentInterface>(_Pawn);
	if (pPawnTeam)
		SetGenericTeamId(pPawnTeam->GetGenericTeamId());
	// 공용헤더에 팀 설정 후 처리
	else
		SetGenericTeamId((uint8)ETeamType::None);

	// 빙의 대상의 스탯 컴포넌트를 가져온다
	UC_EnemyStatComponent* pStatCom = _Pawn->FindComponentByClass<UC_EnemyStatComponent>();

	// 시야 인지범위를 데이터테이블에 작성된 스탯으로 설정
	m_SightConfig->SightRadius = pStatCom->GetStat(TEXT("DetectRange"));
	m_SightConfig->LoseSightRadius = pStatCom->GetStat(TEXT("LoseDetectRange"));

	// 인지 컴포넌트 갱신
	m_PerceptionCom->ConfigureSense(*m_SightConfig);
	m_PerceptionCom->ConfigureSense(*m_DamageConfig);
	m_PerceptionCom->ConfigureSense(*m_HearingConfig);

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
	UE_LOG(LogTemp, Warning, TEXT("Perception Triggered"));

	/*if (!_Target)
		return;

	if (_Stimulus.WasSuccessfullySensed())
	{
		UE_LOG(LogTemp, Warning, TEXT("Detected : %s"), *_Target->GetName());

		if (Blackboard)
		{
			Blackboard->SetValueAsObject(TEXT("Target"), _Target);
		}
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Lost : %s"), *_Target->GetName());

		if (Blackboard)
		{
			Blackboard->ClearValue(TEXT("Target"));
		}
	}*/

	// 감지한 대상이 적인지 아닌지 판단
	AC_Zombie* pZombie = Cast<AC_Zombie>(GetPawn());
	if (nullptr == pZombie)
		return;

	// 감지대상의 우호관계 가져오기
	ETeamAttitude::Type type = pZombie->GetTeamAttitudeTowards(*_Target);

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
		static FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();

		if (_Stimulus.Type == SightID)
		{
			if (pInfo->bSensed)
				pInfo->AggroValue += 10.f;
		}
		else if (_Stimulus.Type == DmgID)
		{
			pInfo->AggroValue += 20.f;
		}
		else
		{
			pInfo->AggroValue += 15.f;
		}
	}
}

void AC_ZombieController::BeginPlay()
{
	Super::BeginPlay();
	
	// 탐지가 발생하면 호출받을 Delegate 등록
	m_PerceptionCom->OnTargetPerceptionUpdated.AddDynamic(this, &AC_ZombieController::OnTargetDetected);
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