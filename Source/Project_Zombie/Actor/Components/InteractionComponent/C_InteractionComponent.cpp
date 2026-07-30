#include "C_InteractionComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Strategy/C_InteractionStrategyBase.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/MeshComponent.h"

#include "TimerManager.h"

#include "Utility/C_Util.h"

#include "Net/UnrealNetwork.h"


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

	// 포커스 대상 있으면 아웃라인 제거
	if (AActor* FocusedTarget = m_FocusedTarget.Get())
	{
		UC_InteractionComponent* TargetInteractionComponent = GetTargetInteractionComponent(FocusedTarget);
		if (TargetInteractionComponent)
		{
			TargetInteractionComponent->SetOutlineEffect(false);
		}
	}

	m_InteractionCandidates.Empty();
	m_FocusedTarget.Reset();
	m_InteractionStrategyObject = nullptr;

	ClearCurrentInteraction();

	Super::EndPlay(EndPlayReason);
}

void UC_InteractionComponent::SetupInteraction(UPrimitiveComponent* _InteractionCollision)
{
	if (!_InteractionCollision)
	{
		UE_LOG(LogTemp, Warning, TEXT("UC_InteractionComponent::SetupInteraction - InteractionSphere is nullptr"));
		return;
	}

	m_InteractionCollision = _InteractionCollision;
	m_CurrentInteractionActor = nullptr;

	// 상호작용 대상 Actor 의 MeshComponent 를 가져와서 아웃라인 효과를 적용할 수 있도록 저장
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->GetComponents<UMeshComponent>(m_OutlineMeshComponents);
	}

	// 상호작용 전략 객체 생성
	CreateInteractionStrategy();

	EnableInteractionDetection();
}

void UC_InteractionComponent::CreateInteractionStrategy()
{
	m_InteractionStrategyObject = nullptr;

	if (!m_InteractionStrategyClass)
		return;

	m_InteractionStrategyObject = NewObject<UC_InteractionStrategyBase>(this, m_InteractionStrategyClass);
	if (!m_InteractionStrategyObject)
	{
		UC_Util::Print("Fail CreateInteractionStrategy", FColor::Red, 10.f);
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
	//if (m_FocusedTarget.Get() == _OtherActor)
	//	m_FocusedTarget.Reset();

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
		if (AActor* OldTarget = m_FocusedTarget.Get())
		{
			UC_InteractionComponent* OldTargetComponent = GetTargetInteractionComponent(OldTarget);
			if (OldTargetComponent)
			{
				OldTargetComponent->SetOutlineEffect(false);
			}
		}

		m_FocusedTarget.Reset();

		StopFocusUpdateTimer();
		return;
	}

	AActor* NewTarget = FindBestInteractionTarget();
	
	if (m_FocusedTarget.Get() == NewTarget)
		return;

	// 포커스 대상이 변경되면 이전 포커스 
	if (AActor* OldTarget = m_FocusedTarget.Get())
	{
		UC_InteractionComponent* OldTargetComponent = GetTargetInteractionComponent(OldTarget);
		if (OldTargetComponent)
		{
			OldTargetComponent->SetOutlineEffect(false);
		}
	}

	m_FocusedTarget = NewTarget;

	if (NewTarget)
	{
		UC_InteractionComponent* NewTargetComponent = GetTargetInteractionComponent(NewTarget);
		if (NewTargetComponent)
		{
			NewTargetComponent->SetOutlineEffect(true);
		}
	}
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

	//m_FocusedTarget.Reset();
}

void UC_InteractionComponent::StartInteractionTimer(float _Duration)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	FTimerManager& TimerManager = World->GetTimerManager();

	TimerManager.ClearTimer(m_InteractionTimerHandle);

	if (_Duration <= 0.f)
	{
		CompleteInteract();
		return;
	}

	TimerManager.SetTimer(
		m_InteractionTimerHandle,
		this,
		&UC_InteractionComponent::CompleteInteract,
		_Duration,
		false
	);
}


bool UC_InteractionComponent::CanBeInteractedBy(AC_BasicPlayer* _Interactor) const
{
	// Actor 가 해당 플레이어에게 상호작용 가능한지 검사
	// Actor 는 GetOwner() 이고, _Interactor 는 상호작용을 시도하는 플레이어

	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	return m_InteractionStrategyObject->CanStartInteraction(_Interactor, GetOwner());
}

float UC_InteractionComponent::GetInteractionDuration() const
{
	return m_InteractionStrategyObject ? m_InteractionStrategyObject->GetInteractionDuration() : 0.f; 
}

void UC_InteractionComponent::TryInteract()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	// 이미 다른 Actor 와 상호작용 중이면 상호작용 불가
	if (IsValid(m_CurrentInteractionActor))
	{
		CancleInteract();
		return;
	}

	AActor* TargetActor = m_FocusedTarget.Get();
	if (!TargetActor)
		return;

	if (TargetActor == m_OwnerPlayer)
		return;

	UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(TargetActor);
	if (!TargetComponent)
		return;


	// 상호작용 대상이 현재 플레이어와 상호작용 가능한지 검사
	// 서버
	if (TargetComponent->GetInteractionNetType() == EInteractionNetType::Server)
	{
		UC_Util::Print("Server Interact", FColor::Red, 10.f);

		if (!m_OwnerPlayer->HasAuthority())
		{
			m_CurrentInteractionActor = TargetActor;
		}

		// 상호작용 시작 시 아웃라인 끄기
		TargetComponent->SetOutlineEffect(false);

		// 서버에서 상호작용 시도 요청
		Server_TryInteract(TargetActor);
		return;
	}

	// 클라이언트
	if (!TargetComponent->ExecuteInteract(m_OwnerPlayer))
		return;

	m_CurrentInteractionActor = TargetActor;

	// 상호작용 시작 시 아웃라인 끄기
	TargetComponent->SetOutlineEffect(false);

	const float InteractionDuration = TargetComponent->GetInteractionDuration();

	UC_Util::Print("Client Interact", FColor::Red, 10.f);

	// 상호작용 완료 타이머는 서버에서 시작되어
	// 서버의 TimerManager 에 의해 CompleteInteract() 호출된다
	StartInteractionTimer(InteractionDuration);
}

void UC_InteractionComponent::CancleInteract()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;

	AActor* TargetActor = m_CurrentInteractionActor.Get();
	if (!TargetActor)
		return;

	UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(TargetActor);
	if (!TargetComponent)
	{
		ClearCurrentInteraction();
		return;
	}

	// 서버 취소
	if (TargetComponent->GetInteractionNetType() == EInteractionNetType::Server)
	{
		Server_CancleInteract(TargetActor);
		
		// 상호작용 취소 시 아웃라인 켜기
		if (m_FocusedTarget.Get() == TargetActor)
		{
			TargetComponent->SetOutlineEffect(true);
		}

		ClearCurrentInteraction();
		
		return;
	}

	// 타이머 취소
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_InteractionTimerHandle);
	}

	// Target 의 Strategy 에게 실제 취소
	TargetComponent->ExecuteCancleInteract(m_OwnerPlayer);

	// 상호작용 취소 시 아웃라인 켜기
	if (m_FocusedTarget.Get() == TargetActor)
	{
		TargetComponent->SetOutlineEffect(true);
	}

	ClearCurrentInteraction();
}

bool UC_InteractionComponent::ExecuteInteract(AC_BasicPlayer* _Interactor)
{
	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	// 이미 다른 플레이어와 상호작용 중이면 상호작용 불가
	if (!m_AllowMultipleInteractor && IsValid(m_CurrentInteractionActor))
	{
		return false;
	}

	// 이미 다른 플레이어와 상호작용 중이면 상호작용 불가
	// 강화 테이블은 여러명이서 사용 가능
	//if (IsValid(m_CurrentInteractionActor))
	//	return false;

	if (!m_InteractionStrategyObject->CanStartInteraction(_Interactor, GetOwner()))
		return false;

	if (!m_InteractionStrategyObject->StartInteraction(_Interactor, GetOwner()))
		return false;

	// 단일 사용자만 저장
	if (!m_AllowMultipleInteractor)
	{
		m_CurrentInteractionActor = _Interactor;
	}

	return true;
}

bool UC_InteractionComponent::ExecuteCancleInteract(AC_BasicPlayer* _Interactor)
{
	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	// 이미 다른 플레이어와 상호작용 중이면 상호작용 불가
	if (!m_AllowMultipleInteractor)
	{
		if (m_CurrentInteractionActor != _Interactor)
			return false;
	}

	m_InteractionStrategyObject->CancleInteraction(_Interactor, GetOwner());

	// 단일 사용자는 상호작용 정보도 초기화
	if (!m_AllowMultipleInteractor)
	{
		ClearCurrentInteraction();
	}

	return true;
}

void UC_InteractionComponent::CompleteInteract()
{
	if (!m_OwnerPlayer)
		return;

	AActor* TargetActor = m_CurrentInteractionActor.Get();
	if (!TargetActor)
	{
		ClearCurrentInteraction();
		return;
	}

	UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(TargetActor);
	if (!TargetComponent)
	{
		ClearCurrentInteraction();
		return;
	}

	TargetComponent->ExecuteCompleteInteract(m_OwnerPlayer);

	ClearCurrentInteraction();
}

bool UC_InteractionComponent::ExecuteCompleteInteract(AC_BasicPlayer* _Interactor)
{
	if (!_Interactor || !m_InteractionStrategyObject)
		return false;

	if (!m_AllowMultipleInteractor)
	{
		if (m_CurrentInteractionActor != _Interactor)
			return false;
	}

	// 대상 쪽 상호작용 완료 처리
	m_InteractionStrategyObject->CompleteInteraction(_Interactor, GetOwner());

	// 대상 쪽 상호작용 정보 초기화
	if (!m_AllowMultipleInteractor)
	{
		ClearCurrentInteraction();
	}

	return true;
}


AActor* UC_InteractionComponent::FindBestInteractionTarget() const
{
	if (!m_OwnerPlayer)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestDot = 0.9f; // Dot Product 범위는 -1.0 ~ 1.0

	// 카메라 위치 기준으로 상호작용 대상 찾기
	FVector ViewLocation;
	FRotator ViewRotation;
	m_OwnerPlayer->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
	
	// 카메라 Forward
	FVector ViewForward = ViewRotation.Vector();
	ViewForward.Normalize();


	// 후보 목록에서 가장 적합한 상호작용 대상 찾기
	for (const TWeakObjectPtr<AActor>& Candidate : m_InteractionCandidates)
	{
		AActor* CandidateActor = Candidate.Get();
		if (!CandidateActor)
			continue;

		// 상호작용 대상의 InteractionComponent 가져오기
		UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(CandidateActor);
		if (!TargetComponent)
			continue;


		// 상호작용 대상이 현재 플레이어와 상호작용 가능한지 검사
		if (!TargetComponent->CanBeInteractedBy(m_OwnerPlayer))
			continue;

		// 벽에 막히면 상호작용 불가
		if (!HasClearLineOfSight(CandidateActor))
			continue;


		FVector DirectionToTarget = CandidateActor->GetActorLocation() - ViewLocation;
		if (!DirectionToTarget.Normalize())
			continue;

		const float Dot = FVector::DotProduct(ViewForward, DirectionToTarget);

		// 화면 중앙 근처만
		if (Dot < 0.85f)
			continue;

		// 현재까지 찾은 대상 중에서 가장 정면에 있는 대상 선택
		if (Dot > BestDot)
		{
			BestDot = Dot;
			BestTarget = CandidateActor;
		}
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

void UC_InteractionComponent::SetOutlineEffect(bool _Enable)
{
	for (UMeshComponent* MeshComp : m_OutlineMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->SetOverlayMaterial(_Enable ? m_OutlineMaterial : nullptr);
		}
	}
}

void UC_InteractionComponent::Server_TryInteract_Implementation(AActor* _TargetActor)
{
	if (!m_OwnerPlayer || !IsValid(_TargetActor))
		return;

	if (_TargetActor == m_OwnerPlayer)
		return;

	// 이미 다른 플레이어와 상호작용 중이면 상호작용 불가
	if (IsValid(m_CurrentInteractionActor))
		return;

	UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(_TargetActor);
	if (!TargetComponent)
		return;

	// 실제 상호작용 시도
	if (!TargetComponent->ExecuteInteract(m_OwnerPlayer))
		return;

	// 성공적으로 상호작용 시작 시도 성공 시 현재 상호작용 중인 Actor 저장
	m_CurrentInteractionActor =	_TargetActor;

	const float InteractionDuration = TargetComponent->GetInteractionDuration();

	UE_LOG(LogTemp, Log, TEXT("Server_TryInteract - Interaction Started with %s for %.2f seconds"), *_TargetActor->GetName(), InteractionDuration);

	// 상호작용 완료 타이머는 서버에서 시작되어
	// 서버의 TimerManager 에 의해 CompleteInteract() 호출된다
	StartInteractionTimer(InteractionDuration);
}

void UC_InteractionComponent::Server_CancleInteract_Implementation(AActor* _TargetActor)
{
	if (!m_OwnerPlayer || !IsValid(_TargetActor))
		return;

	if (m_CurrentInteractionActor.Get() != _TargetActor)
		return;

	UC_InteractionComponent* TargetComponent = GetTargetInteractionComponent(_TargetActor);
	if (!TargetComponent)
		return;

	// 타이머 취소
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_InteractionTimerHandle);
	}

	// Target 의 Strategy 에게 실제 취소
	TargetComponent->ExecuteCancleInteract(m_OwnerPlayer);

	ClearCurrentInteraction();
}
