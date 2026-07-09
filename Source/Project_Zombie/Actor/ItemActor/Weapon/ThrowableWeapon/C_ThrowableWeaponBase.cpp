// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ThrowableWeaponBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameMode/C_UIManager.h"
#include "Interface/I_ExplodeStrategy.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

const FName AC_ThrowableWeaponBase::s_HolsterSocketName = TEXT("ThrowableHolsterSocket");

// Sets default values
AC_ThrowableWeaponBase::AC_ThrowableWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	m_ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	
	m_MainCollider = CreateDefaultSubobject<UCapsuleComponent>("Capsule");
	m_MainCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 직접 투척하기 이전까지는 Collision을 비활성화 처리해주어야 한다
	SetRootComponent(m_MainCollider);

	// 충돌 이벤트 연결
	m_MainCollider->OnComponentHit.AddDynamic(this, &AC_ThrowableWeaponBase::OnThrowableHit);


	// Projectile Movement Component의 움직일 대상을 MainCollider로 설정
	m_ProjectileMovement->SetUpdatedComponent(m_MainCollider);

	// 처음에는 손에 들고 있는 상태이므로 이동 금지
	m_ProjectileMovement->bAutoActivate = false; 

	// 날아가는 방향따라 회전
	m_ProjectileMovement->bRotationFollowsVelocity = true;

	// 중력 적용
	m_ProjectileMovement->ProjectileGravityScale = 1.f; 

	// 튕김 적용
	m_ProjectileMovement->bShouldBounce = true;

	// 튕김 정도
	m_ProjectileMovement->Bounciness = 0.3f; 

	// 튕김 시 마찰 정도 
	m_ProjectileMovement->Friction = 0.7f;

	// 튕김 시 속도가 해당 수치 이하로 떨어지면 더 이상 튕기지 않고 정지 처리
	m_ProjectileMovement->BounceVelocityStopSimulatingThreshold = 30.f;

	// 충돌 활성화 (투척류는 충돌이 있어야 함)
	m_ProjectileMovement->bSweepCollision = true;
	
	// Sub-Stepping 강제 활성화 (투척류는 날아가는 궤적이 직선이 아니므로, Sub-Stepping 활성화 필요)
	m_ProjectileMovement->bForceSubStepping = true; 

	// 몽타주 Section 이름 초기화
	m_RemovePinSectionName = TEXT("RemovePin");
	m_ReadySectionName = TEXT("Ready");
	m_LoopSectionName = TEXT("Loop");
	m_ThrowSectionName = TEXT("Throw");
	
	// 투척류 Launch 위치 Offset 초기화
	m_LaunchUpwardOffset = 10.f;
	m_LaunchForwardOffset = 50.f;

	m_ThrowSpeed = 1500.f;

	// 투척류 상태 초기화
	ResetThrowableState();

	// TODO : PathSpline으로 예측 경로 그리기 처리 시, SplineComponent 및 PredictedEndPoint StaticMesh 또한 CreateDefaultSubobject로 생성해줄 것
	// TODO : Explosion Sphere (폭발 반경 Sphere) 는 만들어주어야 함
}

// Called when the game starts or when spawned
void AC_ThrowableWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	m_ProjectileMovement->Deactivate();

	// 폭발 전략 클래스가 지정되어 있다면, 해당 클래스의 객체를 생성
	if (m_ExplodeStrategyClass)
	{
		if (m_ExplodeStrategyClass->ImplementsInterface(UI_ExplodeStrategy::StaticClass()))
		{
			m_ExplodeStrategyObject = NewObject<UObject>(this, m_ExplodeStrategyClass);
		}
	}
}

// Called every frame
void AC_ThrowableWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AC_ThrowableWeaponBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	
	// 투척류를 장착하는 경우, 투척류 상태 초기화
	ResetThrowableState();

	SetActorHiddenInGame(false);

	m_ProjectileMovement->Deactivate();
	// ClearSpline(); // PathSpline 예측 경로 기능이 있다 하면, Clear 한번 여기서 처리를 해주어야 함

	// Self init (이 변수들 처리 추후, 던지기 기다리기 처리 시 필요함)
	// bIsCharging       = false;
	// bIsOnThrowProcess = false;

	// Main HUD Throwable 종류로 초기화
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(Player->GetController<APlayerController>()->GetHUD()))
		UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, EFireMode::Single, 1, 1);

	// 이 처리는 왜 해줬는지 잘 기억은 안남 (아마 Attach 하기전에 처리를 해주어야 똑바로 위치처리가 되어서 해주었던 것 같음)
	// TODO : 필요하다면 그때가서 풀기 (아마 딱히 필요 없어보임)
	// SetActorRelativeLocation(FVector::ZeroVector);
	
	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HandSocketName
	);
	
	if (bIsAttached)
		Player->SetHandState(EHandState::WeaponThrowable);
	
	return bIsAttached;
}

bool AC_ThrowableWeaponBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	if (!Cast<AC_BasicPlayer>(_ParentMesh->GetOwner())) return false; // 무기집에 붙이려는 Actor가 Player형이 아닌 경우
	
	// 배그 모작에서,
	// 투척류를 핀까지만 뽑았고 쿠킹을 안했을 시 다시 집어넣음
	// 투척류를 안전손잡이까지 뽑았다면 현재 위치에 현재 투척류 그냥 바닥에 떨굼

	// 투척류를 집어넣는 경우, 투척류 상태 초기화
	CancleThrowAction();

	SetActorHiddenInGame(true);
	m_ProjectileMovement->Deactivate();

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		s_HolsterSocketName
	);
	return bIsAttached;
}

bool AC_ThrowableWeaponBase::OnStartFire(class AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser)
		return false;

	if (m_ThrowableState != EThrowableState::Idle)
	{
		return false;
	}

	if (_WeaponUser->GetMesh()->GetAnimInstance()->Montage_IsPlaying(m_ThrowMontage))
	{
		return false;
	}

	// 무기 사용자를 저장해둠 (애님 노티파이 이벤트에서 사용하기 위함)
	m_WeaponUser = _WeaponUser;

	// 투척 과정 시작
	m_bIsCharging = false;
	m_bWantsCook = false;
	
	// 핀이 있는 투척류면 핀 제거 동작 / 핀이 없는 투척류면 바로 차징 동작 부터 시작
	const FName StartSectionName = m_bHasPin ? m_RemovePinSectionName : m_ReadySectionName;

	m_ThrowableState = m_bHasPin ? EThrowableState::RemovePin : EThrowableState::Ready;
	 
	// 투척류 애니메이션 재생
	_WeaponUser->PlayAnimMontage(m_ThrowMontage, 1.f, StartSectionName);

	return true;
}

bool AC_ThrowableWeaponBase::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser)
		return false;

	if (m_WeaponUser != _WeaponUser)
		m_WeaponUser = _WeaponUser;

	return OnStartCookInput();
}

bool AC_ThrowableWeaponBase::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser)
		return false;

	m_bIsCharging = true;

	return true;
}

bool AC_ThrowableWeaponBase::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser)
		return false;

	m_bIsCharging = false;

	return true;
}

// ----------------- 애님 노티파이 관련 처리 -----------------

void AC_ThrowableWeaponBase::OnRemovePin()
{
	if (!m_bHasPin)
		return;

	m_ThrowableState = EThrowableState::Ready;

	// R 키를 먼저 눌러둔 경우, 핀 제거 후 바로 타이머 시작
	if (m_bWantsCook)
	{
		StartFuseTimer();
	}
}

void AC_ThrowableWeaponBase::OnThrowReadyLoop()
{
	UAnimInstance* AnimInstance = m_WeaponUser->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;
	
	// 차징 중이면, ReadyLoop 상태로 넘어가고 투척 동작으로 넘어가지 않음
	if (m_bIsCharging)
	{
		// TODO : 차징 중이면 예상 경로를 그려주기
		m_ThrowableState = EThrowableState::ReadyLoop;
		return;
	}

	// 차징이 끝났으면, 투척 동작으로 넘어감
	m_ThrowableState = EThrowableState::Throwing;

	m_WeaponUser->PlayAnimMontage(m_ThrowMontage, 1.f, m_ThrowSectionName);
}

void AC_ThrowableWeaponBase::OnThrowThrowable()
{
	if (!m_WeaponUser)
		return;

	if (!m_MainCollider || !m_ProjectileMovement)
		return;

	// 투척 방향과 투척 시작 위치 계산
	const FVector ThrowDirection = GetThrowDirection();
	const FVector LaunchLocation = GetLaunchLocation(ThrowDirection);
	const FRotator LaunchRotation = ThrowDirection.Rotation();

	// 현재 붙어있는 손 소켓에서 분리하고 월드 Transform 은 유지
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	// 손보다 앞에서 투척 시작
	SetActorLocationAndRotation
	(
		LaunchLocation,
		LaunchRotation,
		false, // 이동 경로 충돌 검사			// 이거 true로 하면 투척류가 손에서 분리될 때, 손과 충돌해서 튕겨나가는 현상 발생
		nullptr, // 충돌 정보 받을 포인터
		ETeleportType::TeleportPhysics
	);

	// 투척류 Projectile Movement 활성화
	LaunchCurrentActorAsProjectile(ThrowDirection);

	m_ThrowableState = EThrowableState::Thrown;
	m_bIsCharging = false;

	// 타이머형 투척류만 타이머 시작
	if (!m_bExplodeOnImpact && HasFuseTimer())
	{
		StartFuseTimer();
	}

	// TODO 
	// 수류탄 던짐
	// EquippedComponent의 CurrentWeapon은 nullptr 또는 다음 수류탄으로 변경
	// 수류탄 개수 감소
}

// ----------------- 쿠킹 관련 처리 -----------------

bool AC_ThrowableWeaponBase::OnStartCookInput()
{
	// 쿠킹 불가한 경우
	if (!m_bIsCookable)
		return false;

	// 타이머가 없는 경우 쿠킹 불가
	if (!HasFuseTimer())
		return false;

	// 핀 제거 전 단계는 쿠킹 불가
	if (m_ThrowableState == EThrowableState::Idle || m_ThrowableState == EThrowableState::RemovePin)
		return false;

	UC_Util::Print("OnStartCookInput");

	return StartFuseTimer();
}

void AC_ThrowableWeaponBase::Explode()
{
	// 폭발 처리는 II_ExplodeStrategy를 상속받은 클래스에서 처리할 예정
	if (m_ThrowableState == EThrowableState::Exploded)
		return;

	const EThrowableState PrevState = m_ThrowableState;

	/// TODO : 폭발 시, 투척류 애님 몽타주가 재생 중이면 Stop 처리
	/// 수류탄을 들고있는 기본 상태보다 아무것도 들고있지 않는 기본 상태로
	if (PrevState != EThrowableState::Thrown && PrevState != EThrowableState::Throwing)
	{
		UAnimInstance* AnimInstance = m_WeaponUser->GetMesh()->GetAnimInstance();

		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.2f, m_ThrowMontage);
		}
	}

	m_ThrowableState = EThrowableState::Exploded;

	// 타이머 초기화
	ClearFuseTimer();

	// 이동 정지
	if (m_ProjectileMovement)
	{
		m_ProjectileMovement->StopMovementImmediately(); // 속도 제거
		m_ProjectileMovement->Deactivate(); // Projectile Movement 비활성화
	}

	// 충돌 비활성화
	if (m_MainCollider)
	{
		m_MainCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);

	// 해당 객체가 Interface를 구현했는지 확인
	if (!m_ExplodeStrategyObject->GetClass()->ImplementsInterface(UI_ExplodeStrategy::StaticClass()))
	{
		UC_Util::Print("[AC_ThrowableWeaponBase::Explode] Not Implements Interface");
		
		SetActorHiddenInGame(true);
		Destroy();
		return;
	}

	bool bExploded = II_ExplodeStrategy::Execute_UseStrategy(m_ExplodeStrategyObject, this);

	if (bExploded)
	{
		// 폭발 처리 완료 후, Actor 제거
	}

	SetActorHiddenInGame(true);
	Destroy();
}

// ----------------- 투척 취소 관련 처리 -----------------

void AC_ThrowableWeaponBase::CancleThrowAction()
{
	if (!m_WeaponUser)
		return;

	UAnimInstance* AnimInstance = m_WeaponUser->GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		// 차징되어 Pause 된 상태라면, Resume 후 Stop 처리
		if (m_ThrowableState == EThrowableState::ReadyLoop)
		{
			AnimInstance->Montage_Resume(m_ThrowMontage);
		}

		// 투척 동작 취소 처리
		AnimInstance->Montage_Stop(0.2f, m_ThrowMontage);
	}

	/// TODO : 타이머 취소 처리

	ResetThrowableState();
}

void AC_ThrowableWeaponBase::ResetThrowableState()
{
	m_ThrowableState = EThrowableState::Idle;

	m_bIsCharging = false;
	m_bWantsCook = false;

	m_WeaponUser = nullptr;
}


// ----------------- 투척 관련 처리 -----------------

FVector AC_ThrowableWeaponBase::GetThrowDirection() const
{
	if (!m_WeaponUser)
		return GetActorForwardVector();

	// 플레이어가 바라보는 방향을 기준으로 투척 방향 계산
	FVector ThrowDirection = m_WeaponUser->GetActorForwardVector();

	// 마우스 방향을 기준으로 투척 방향 계산
	if (AController* Controller = m_WeaponUser->GetController())
	{
		FRotator ControlRotation = Controller->GetControlRotation();
		ThrowDirection = ControlRotation.Vector();
	}

	// 투척류를 들고 있는 위치에서 약간 위로 보정
	ThrowDirection += FVector::UpVector * 0.15f;

	return ThrowDirection.GetSafeNormal();
}

FVector AC_ThrowableWeaponBase::GetLaunchLocation(const FVector& _ThrowDirection) const
{
	FVector LaunchLocation = GetActorLocation();

	if (m_WeaponUser)
	{
		// 캐릭터가 바라보는 방향을 기준으로 투척 시작 위치 계산
		const FVector CharacterForard = m_WeaponUser->GetActorForwardVector().GetSafeNormal();

		LaunchLocation += CharacterForard * m_LaunchForwardOffset;
		LaunchLocation += FVector::UpVector * m_LaunchUpwardOffset;
	}
	else
	{
		// 투척 방향을 기준으로 투척 시작 위치 계산
		LaunchLocation += _ThrowDirection * m_LaunchForwardOffset;
		LaunchLocation += FVector::UpVector * m_LaunchUpwardOffset;
	}

	return LaunchLocation;
}

void AC_ThrowableWeaponBase::SetupThrowCollision()
{
	// 이 Actor 는 충돌을 할거야
	SetActorEnableCollision(true);

	// 이 Actor의 모든 PrimitiveComponent를 가져옴
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Component : PrimitiveComponents)
	{
		// Visibility 활성화
		Component->SetVisibility(true, true);
		// Hidden 상태 해제
		Component->SetHiddenInGame(false, true);

		// MainCollider 만 충돌 활성화, 나머지 Collider는 충돌 비활성화 처리
		if (Component != m_MainCollider)
		{
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	if (!m_MainCollider)
		return;

	// MainCollider 충돌 활성화
	m_MainCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_MainCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); 

	// 일단 모든 채널에 Block
	// TODO: 나중에 Projectile Channel을 만들어야?
	m_MainCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	// Hit 이벤트 발생
	m_MainCollider->SetNotifyRigidBodyCollision(true);

	// ProjectileMovement 로 날아가니까 Physics Simulation은 끄기
	m_MainCollider->SetSimulatePhysics(false);

	if (m_WeaponUser)
	{
		// Owner와 충돌하지 않도록 설정
		m_MainCollider->IgnoreActorWhenMoving(m_WeaponUser, true);
		m_WeaponUser->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
	}
}

void AC_ThrowableWeaponBase::LaunchCurrentActorAsProjectile(const FVector& _ThrowDirection)
{
	// 숨김 해제
	SetActorHiddenInGame(false);

	// 충돌 활성화
	SetupThrowCollision();

	if (!m_ProjectileMovement)
		return;

	// ProjectileMovementComponent 활성화
	if (m_MainCollider)
	{
		m_ProjectileMovement->SetUpdatedComponent(m_MainCollider);
	}

	// 현재 이동 정지 (기존 속도 제거)
	m_ProjectileMovement->StopMovementImmediately();

	if (m_ThrowSpeed <= 0.f)
	{
		m_ThrowSpeed = 1500.f; // 기본 투척 속도 설정
	}

	if (m_bExplodeOnImpact)
	{
		// 충돌 시 폭발 처리이므로 튕김 비활성화
		m_ProjectileMovement->bShouldBounce = false; 
	}
	else
	{
		// 충돌해도 복발 안하므로 튕김 활성화
		m_ProjectileMovement->bShouldBounce = true; 
		m_ProjectileMovement->Bounciness = 0.3f;
		m_ProjectileMovement->Friction = 0.7f;
		m_ProjectileMovement->BounceVelocityStopSimulatingThreshold = 30.f;
	}

	// 투척 속도 설정
	m_ProjectileMovement->InitialSpeed = m_ThrowSpeed;
	m_ProjectileMovement->MaxSpeed = FMath::Max(m_ProjectileMovement->MaxSpeed, m_ThrowSpeed);

	// 방향 설정
	m_ProjectileMovement->Velocity = _ThrowDirection.GetSafeNormal() * m_ThrowSpeed;

	// 실행
	m_ProjectileMovement->Activate(true);
}

void AC_ThrowableWeaponBase::OnThrowableHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 던져진 상태가 아니면 Hit 이벤트 무시
	if (m_ThrowableState != EThrowableState::Thrown)
		return;
	
	// 혹시 투척류를 던진 플레이어와 충돌했을 경우 손에서 터질 수 있으므로 충돌 무시
	if (OtherActor == m_WeaponUser)
		return;

	// 충돌 시 폭발 하도록 설정되어 있지 않으면 무시
	if (!m_bExplodeOnImpact)
		return;

	Explode();
}

// --------------- 타이머 관련 ------------------


bool AC_ThrowableWeaponBase::HasFuseTimer() const
{
	// FuseTime이 0보다 크면 타이머가 있는 것으로 간주
	return m_FuseTime > 0.f;
}

bool AC_ThrowableWeaponBase::StartFuseTimer()
{
	if (!HasFuseTimer())
		return false;

	// 타이머 설정
	UWorld* World = GetWorld();
	
	// 이미 타이머가 활성화되어 있다면, 중복 설정 방지
	if (World->GetTimerManager().IsTimerActive(m_FuseTimerHandle))
	{
		return true;
	}
	
	m_bWantsCook = false; // 쿠킹 시작했으므로 WantsCook 초기화

	World->GetTimerManager().SetTimer
	(
		m_FuseTimerHandle, 
		this, 
		&AC_ThrowableWeaponBase::OnFuseTimerFinished,
		m_FuseTime,
		false
	);
	
	UC_Util::Print("Start Fuse Timer");

	return true;
}

void AC_ThrowableWeaponBase::ClearFuseTimer()
{
	// 타이머 취소
	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(m_FuseTimerHandle);

	m_bWantsCook = false; // 쿠킹 취소했으므로 WantsCook 초기화
}

void AC_ThrowableWeaponBase::OnFuseTimerFinished()
{
	UC_Util::Print("Finish Fuse Timer");

	Explode();
}
