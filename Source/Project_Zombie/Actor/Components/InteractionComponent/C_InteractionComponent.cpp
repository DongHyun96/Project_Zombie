#include "C_InteractionComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Strategy/C_InteractionStrategyBase.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"

#include "TimerManager.h"

#include "Utility/C_Util.h"



UC_InteractionComponent::UC_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Replication
	SetIsReplicatedByDefault(true);
}

void UC_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 소유한 Actor 가 Player 인지 확인
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());

	// 상호작용 전략 객체 생성
	CreateInteractionStrategy();
}

void UC_InteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Overlap 이벤트 제거
	if (m_InteractionCollision)
	{
		m_InteractionCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UC_InteractionComponent::OnInteractionBeginOverlap);
		m_InteractionCollision->OnComponentEndOverlap.RemoveDynamic(this, &UC_InteractionComponent::OnInteractionEndOverlap);
	}

	m_InteractionCandidates.Empty();
	m_InteractionStrategyObject = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UC_InteractionComponent::SetupInteraction(UPrimitiveComponent* _InteractionCollision, TSubclassOf<UC_InteractionStrategyBase> _StrategyClass)
{
	if (!_InteractionCollision)
	{
		UE_LOG(LogTemp, Warning, TEXT("UC_InteractionComponent::SetupInteraction - InteractionSphere is nullptr"));
		return;
	}

	m_InteractionCollision = _InteractionCollision;
	m_InteractionStrategyClass = _StrategyClass;

	// BP BeginPlay 에서 호출된 경우 컴포넌트 BeginPlay() 가 이미 호출되었을 수 있으므로 여기서 전략 객체를 생성
	if (HasBegunPlay())
	{
		CreateInteractionStrategy();
	}
}

void UC_InteractionComponent::CreateInteractionStrategy()
{
	m_InteractionStrategyObject = nullptr;

	if (!m_InteractionStrategyClass)
		return;

	m_InteractionStrategyObject = NewObject<UC_InteractionStrategyBase>(this, m_InteractionStrategyClass);
	if (!m_InteractionStrategyObject)
	{
		// 전략 객체 생성 실패
	}
}

void UC_InteractionComponent::EnableInteractionDetection()
{
	// 감지하는 기능은 로컬 플레이어에서만
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	if (!m_InteractionCollision)
		return;

	m_InteractionCollision->OnComponentBeginOverlap.AddDynamic(this, &UC_InteractionComponent::OnInteractionBeginOverlap);
	m_InteractionCollision->OnComponentEndOverlap.AddDynamic(this, &UC_InteractionComponent::OnInteractionEndOverlap);
}


void UC_InteractionComponent::OnInteractionBeginOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _bFromSweep, const FHitResult& _SweepResult)
{
	// 로컬 플레이어에서만 처리
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	// 상호작용 대상이 자기 자신이면 무시
	if (_OtherActor == GetOwner())
		return;

	// 상호작용 대상이 InteractionCollision 태그를 가지고 있는지 확인
	if (!_OtherComp->ComponentHasTag(TEXT("InteractionCollision")))
		return;

	// 후보 목록에 Actor 추가
	// 등록되지 않은 Actor 만 후보 목록에 추가
	if (GetTargetInteractionComponent(_OtherActor))
	{
		m_InteractionCandidates.AddUnique(_OtherActor);
	}
}

void UC_InteractionComponent::OnInteractionEndOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex)
{
	// 로컬 플레이어에서만 처리
	if (!m_OwnerPlayer->IsLocallyControlled())
		return;

	// 상호작용 대상이 InteractionCollision 태그를 가지고 있는지 확인
	if (!_OtherComp->ComponentHasTag(TEXT("InteractionCollision")))
		return;

	// 후보 목록에서 Actor 제거
	if (GetTargetInteractionComponent(_OtherActor))
	{
		m_InteractionCandidates.Remove(_OtherActor);
	}
}


bool UC_InteractionComponent::CanBeInteractedBy(AC_BasicPlayer* _Interactor) const
{
	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	return m_InteractionStrategyObject->CanStartInteraction(_Interactor, GetOwner());
}


AActor* UC_InteractionComponent::FindBestInteractionTarget() const
{
	if (!m_OwnerPlayer)
		return nullptr;

	AActor* BestTarget = nullptr;

	// 후보 목록에서 가장 적합한 상호작용 대상 찾기
	for (const TWeakObjectPtr<AActor>& Candidate : m_InteractionCandidates)
	{
		AActor* CandidateActor = Candidate.Get();
		if (!CandidateActor)
			continue;

		UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(CandidateActor);
	}

	// 벽에 막히면 상호작용 불가
	// HasClearLineOfSight

	return nullptr;
}

UC_InteractionComponent* UC_InteractionComponent::GetTargetInteractionComponent(AActor* _TargetActor) const
{
	return nullptr;
}

bool UC_InteractionComponent::HasClearLineOfSight(AActor* _TargetActor) const
{
	return false;
}
