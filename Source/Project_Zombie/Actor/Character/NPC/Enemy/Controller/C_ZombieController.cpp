// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieController.h"

// PerceptionComponent
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"

#include "../C_ZombieStatComponent.h"

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
		m_SightConfig->DetectionByAffiliation.bDetectFriendlies = false; // 감지대상이 우호관계인경우 탐지한것으로 인정
		m_SightConfig->DetectionByAffiliation.bDetectNeutrals = false; // 감지대상이 중립관계인경우 탐지한것으로 인정

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


	// 탐지가 발생하면 호출받을 Delegate 등록
	m_PerceptionCom->OnTargetPerceptionUpdated.AddDynamic(this, &AC_ZombieController::OnTargetDetected);
}

void AC_ZombieController::OnPossess(APawn* _Pawn)
{
	Super::OnPossess(_Pawn);

	// 빙의한 대상과 같은 팀으로 설정
	const IGenericTeamAgentInterface* pPawnTeam = Cast<IGenericTeamAgentInterface>(_Pawn);
	if (pPawnTeam)
		SetGenericTeamId(pPawnTeam->GetGenericTeamId());
	// 공용헤더에 팀 설정 후 처리
	//else
	//	SetGenericTeamId((uint8)ETeamType::None);

	// 빙의 대상의 스탯 컴포넌트를 가져온다
	UC_ZombieStatComponent* pStatCom = _Pawn->FindComponentByClass<UC_ZombieStatComponent>();

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
	if (!_Target)
		return;

	// 감지한 대상이 적인지 아닌지 판단
	AC_Zombie* pZombie = Cast<AC_Zombie>(GetPawn());
	if (nullptr == pZombie)
		return;
}
