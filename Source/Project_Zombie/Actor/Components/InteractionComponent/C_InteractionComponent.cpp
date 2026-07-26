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

	// 타이머 제거
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(m_FocusUpdateTimerHandle);
	}

	m_InteractionCandidates.Empty();
	m_FocusedTarget.Reset();
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

	m_InteractionCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &UC_InteractionComponent::OnInteractionBeginOverlap);
	m_InteractionCollision->OnComponentEndOverlap.AddUniqueDynamic(this, &UC_InteractionComponent::OnInteractionEndOverlap);
}


void UC_InteractionComponent::OnInteractionBeginOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _bFromSweep, const FHitResult& _SweepResult)
{
	// 로컬 플레이어에서만 처리
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;
	
	if (!_OtherActor || !_OtherComp)
		return;

	// 상호작용 대상이 자기 자신이면 무시
	if (_OtherActor == GetOwner())
		return;

	// 상호작용 대상이 I_Interactable 인터페이스를 구현하고 있는지 확인
	if (!GetTargetInteractionComponent(_OtherActor))
		return;

	// 후보 목록에 Actor 추가
	// 등록되지 않은 Actor 만 후보 목록에 추가
	m_InteractionCandidates.AddUnique(_OtherActor);
	
	// 가장 적합한 상호작용 대상 업데이트 시작
	UpdateFocusedTarget();
	StartFocusUpdateTimer();
}

void UC_InteractionComponent::OnInteractionEndOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex)
{
	// 로컬 플레이어에서만 처리
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	if (!_OtherActor)
		return;

	// 후보 목록에서 Actor 제거
	m_InteractionCandidates.Remove(_OtherActor);

	// 제거한 Actor 가 현재 포커스 대상이면 포커스 대상 초기화
	if (m_FocusedTarget.Get() == _OtherActor)
		m_FocusedTarget.Reset();

	// 남은 후보 중에 가장 적합한 상호작용 대상 업데이트
	UpdateFocusedTarget(); 
}

void UC_InteractionComponent::UpdateFocusedTarget()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
	{
		StopFocusUpdateTimer();
		return;
	}

	// TODO: Actor 가 Destroy 됐는데 후보 목록에 남아있을 수 있으므로 유효성 검사 후 제거

	if (m_InteractionCandidates.Num() == 0)
	{
		m_FocusedTarget.Reset();
		StopFocusUpdateTimer();
		return;
	}

	m_FocusedTarget = FindBestInteractionTarget();
}

void UC_InteractionComponent::StartFocusUpdateTimer()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	if (m_InteractionCandidates.Num() == 0)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	FTimerManager& TimerManager = World->GetTimerManager();

	// 이미 타이머 활성화 되어 있으면 중복 설정 방지
	if (TimerManager.IsTimerActive(m_FocusUpdateTimerHandle))
		return;

	TimerManager.SetTimer(
		m_FocusUpdateTimerHandle,
		this,
		&UC_InteractionComponent::UpdateFocusedTarget,
		m_FocusUpdateInterval,
		true
	);
}

void UC_InteractionComponent::StopFocusUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_FocusUpdateTimerHandle);
	}

	m_FocusedTarget.Reset();
}


bool UC_InteractionComponent::CanBeInteractedBy(AC_BasicPlayer* _Interactor) const
{
	// Actor 가 해당 플레이어에게 상호작용 가능한지 검사
	// Actor 는 GetOwner() 이고, _Interactor 는 상호작용을 시도하는 플레이어

	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	return m_InteractionStrategyObject->CanStartInteraction(_Interactor, GetOwner());
}

void UC_InteractionComponent::TryInteract()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	AActor* TargetActor = m_FocusedTarget.Get();
	if (!TargetActor)
		return;

	UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(TargetActor);
	if (!TargetComponent)
		return;

	TargetComponent->ExecuteInteract(m_OwnerPlayer);
}

bool UC_InteractionComponent::ExecuteInteract(AC_BasicPlayer* _Interactor)
{
	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	if (!m_InteractionStrategyObject->CanStartInteraction(_Interactor, GetOwner()))
		return false;

	return m_InteractionStrategyObject->StartInteraction(_Interactor, GetOwner());
}


AActor* UC_InteractionComponent::FindBestInteractionTarget() const
{
	if (!m_OwnerPlayer)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDot = -2.0f; // Dot Product 범위는 -1.0 ~ 1.0

	// 컨트롤러 방향으로 체크
	FVector ViewForward = m_OwnerPlayer->GetControlRotation().Vector();
	ViewForward.Z = 0.0f; // 수평 방향만 고려
	ViewForward.Normalize();

	const FVector SourceLocation = m_InteractionCollision->GetComponentLocation();

	// 후보 목록에서 가장 적합한 상호작용 대상 찾기
	for (const TWeakObjectPtr<AActor>& Candidate : m_InteractionCandidates)
	{
		AActor* CandidateActor = Candidate.Get();
		if (!CandidateActor)
			continue;

		// 상호작용 대상의 InteractionComponent 가져오기
		UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(CandidateActor);
		if (!TargetComponent || !TargetComponent->m_InteractionCollision)
			continue;

		// 상호작용 대상이 현재 플레이어와 상호작용 가능한지 검사
		if (!TargetComponent->CanBeInteractedBy(m_OwnerPlayer))
			continue;

		// 벽에 막히면 상호작용 불가
		if (!HasClearLineOfSight(CandidateActor))
			continue;


		FVector DirectionToTarget = TargetComponent->m_InteractionCollision->GetComponentLocation() - SourceLocation;
		DirectionToTarget.Z = 0.0f; // 수평 방향만 고려

		if (!DirectionToTarget.Normalize())
			continue;

		const float Dot = FVector::DotProduct(ViewForward, DirectionToTarget);

		// 정면에 있는 대상 중에
		if (Dot <= 0.0f)
			continue;

		// 현재까지 찾은 대상 중에서 가장 정면에 있는 대상 선택
		if (Dot > BestDot)
		{
			BestDot = Dot;
			BestTarget = CandidateActor;
		}

		UC_Util::Print("Find BestTarget", FColor::Red, 10.f);
	}


	return BestTarget;
}

UC_InteractionComponent* UC_InteractionComponent::GetTargetInteractionComponent(AActor* _TargetActor) const
{
	if (!IsValid(_TargetActor))
		return nullptr;

	// 상호작용 대상이 I_Interactable 인터페이스를 구현하고 있는지 확인
	II_Interactable* InteractableActor = Cast<II_Interactable>(_TargetActor);
	if (!InteractableActor)
		return nullptr;

	// 인터페이스는 구현했지만 InteractionComponent 를 가지고 있는지 확인
	UC_InteractionComponent* TargetComponent = InteractableActor->GetInteractionComponent();
	if (!TargetComponent)
		return nullptr;

	return TargetComponent;
}

bool UC_InteractionComponent::HasClearLineOfSight(AActor* _TargetActor) const
{
	if (!m_OwnerPlayer || !IsValid(_TargetActor))
		return false;

	UWorld* World = GetWorld();
	if (!World)
		return false;

	const FVector TraceStart = m_OwnerPlayer->GetPawnViewLocation(); // 카메라 기준 위치
	const FVector TraceEnd = _TargetActor->GetActorLocation();

	FCollisionQueryParams QueryParams
	(
		SCENE_QUERY_STAT(InteractionLineOfSight),
		false
	);

	QueryParams.AddIgnoredActor(m_OwnerPlayer); // 플레이어 자신은 무시
	QueryParams.AddIgnoredActor(_TargetActor); // 상호작용 컴포넌트 소유 Actor 무시

	const bool bBlocked = World->LineTraceTestByChannel(
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	return !bBlocked;
}
