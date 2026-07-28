// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ThrowableWeaponBase.h"

#include "NiagaraSystem.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Area/C_FireDamageArea.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

#include "GameModeAndManager/C_UIManager.h"

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

	// Create StaticMeshComponent,
	m_WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	m_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_WeaponMesh->SetupAttachment(RootComponent);
	
	// 충돌 이벤트 연결
	m_MainCollider->OnComponentHit.AddDynamic(this, &AC_ThrowableWeaponBase::OnThrowableHit);

	// ===========================
	// Projectile Movement Component 초기화
	// ===========================

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



	// TODO : PathSpline으로 예측 경로 그리기 처리 시, SplineComponent 및 PredictedEndPoint StaticMesh 또한 CreateDefaultSubobject로 생성해줄 것
	// TODO : Explosion Sphere (폭발 반경 Sphere) 는 만들어주어야 함
	// ===========================
	// 예상 경로 초기화
	// ===========================

	m_PathSpline = CreateDefaultSubobject<USplineComponent>("PathSpline");
	m_PathSpline->SetupAttachment(m_MainCollider);

	// 조준 방향이 변경될 때마다 Spline이 따라 움직일 수 있도록 Mobility를 Movable로 설정
	m_PathSpline->SetMobility(EComponentMobility::Movable);

	m_PredictedEndPoint = CreateDefaultSubobject<UStaticMeshComponent>("PredictedEndPoint");
	m_PredictedEndPoint->SetupAttachment(m_MainCollider);

	// 예측 경로는 충돌 비활성화
	m_PredictedEndPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	// 예측 경로는 Overlap 이벤트 비활성화
	m_PredictedEndPoint->SetGenerateOverlapEvents(false);
	// 예측 경로는 처음에는 안보이게
	m_PredictedEndPoint->SetVisibility(false);
	// 예측 경로는 그림자 안보이게
	m_PredictedEndPoint->SetCastShadow(false);



	// 몽타주 Section 이름 초기화
	m_RemovePinSectionName = TEXT("RemovePin");
	m_ReadySectionName = TEXT("Ready");
	m_LoopSectionName = TEXT("Loop");
	m_ThrowSectionName = TEXT("Throw");
	
	// 투척류 Launch 위치 Offset 초기화
	m_LaunchUpwardOffset = 10.f;
	m_LaunchForwardOffset = 50.f;

	m_ThrowSpeed = 1500.f;

	m_ExplosionEffectScale = 1.0f;

	// 투척류 상태 초기화
	ResetThrowableState();
	
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

bool AC_ThrowableWeaponBase::InitializeItemActor(const FWeaponData* InRawData)
{
	//return Super::InitializeItemActor(InRawData);
	
	const FThrowableData* ThrowableData = static_cast<const FThrowableData*>(InRawData);
	
	if (!ThrowableData)
	{
		UC_Util::Print("Failed Cast to const FThrowableData*", FColor::Red, 10.f);
		return false;
	}
	
	// 기본 수치 및 플래그 설정 (서버 로직)
	m_bExplodeOnImpact     = ThrowableData->m_bExplodeOnImpact;
	m_bHasPin              = ThrowableData->m_bHasPin;
	m_bIsCookable         = ThrowableData->m_bIsCookable;
	m_ExplosionEffectScale = ThrowableData->m_ExplosionEffectScale;
	m_ExplosionRadius      = ThrowableData->m_ExplosionRadius;
	m_FuseTime             = ThrowableData->m_FuseTime;
	m_MaxDamage           = ThrowableData->m_MaxDamage;
	m_MinDamage           = ThrowableData->m_MinDamage;

	if (!ItemLinkComp)
	{
		UC_Util::Print("ThrowableBase : Item Link Component Is Nullptr!", FColor::Red, 10.f);
	}

	// 비동기 에셋 로드 호출
	LoadAsyncAssets(ThrowableData);
	return true;
}

void AC_ThrowableWeaponBase::LoadAsyncAssets(const FWeaponData* InRawData)
{
	//Super::LoadAsyncAssets(InRawData);

    const FThrowableData* ThrowableData = static_cast<const FThrowableData*>(InRawData);
    if (!ThrowableData)
    {
       UC_Util::Print("Failed Cast to const FThrowableData*", FColor::Red, 10.f);
       return;
    }

    // 기존 로딩 핸들 취소 및 정리 (오타 수정: CancelAsyncLoad)
    CancelAsyncLoad();

    // 비동기로 로드할 SoftPath 목록 수집
    TArray<FSoftObjectPath> AssetsToLoad;

    if (!ThrowableData->WeaponStaticMesh.IsNull())        AssetsToLoad.Add(ThrowableData->WeaponStaticMesh.ToSoftObjectPath());
    if (!ThrowableData->m_ThrowMontage.IsNull())          AssetsToLoad.Add(ThrowableData->m_ThrowMontage.ToSoftObjectPath());
    if (!ThrowableData->m_ExplodeStrategyClass.IsNull())  AssetsToLoad.Add(ThrowableData->m_ExplodeStrategyClass.ToSoftObjectPath());
    if (!ThrowableData->m_ExplosionEffect.IsNull())        AssetsToLoad.Add(ThrowableData->m_ExplosionEffect.ToSoftObjectPath());
    if (!ThrowableData->m_FireDamageAreaClass.IsNull())   AssetsToLoad.Add(ThrowableData->m_FireDamageAreaClass.ToSoftObjectPath());

    if (AssetsToLoad.Num() > 0)
    {
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

        // 람다 안전 캡처용 SoftPointer 복사
        TSoftObjectPtr<UStaticMesh>     SoftMesh            = ThrowableData->WeaponStaticMesh;
        TSoftObjectPtr<UAnimMontage>    SoftThrowMontage    = ThrowableData->m_ThrowMontage;
        TSoftClassPtr<UObject>          SoftStrategyClass   = ThrowableData->m_ExplodeStrategyClass;
        TSoftObjectPtr<UParticleSystem>  SoftExplosionEffect = ThrowableData->m_ExplosionEffect;
        TSoftClassPtr<AC_FireDamageArea>           SoftDamageAreaClass = ThrowableData->m_FireDamageAreaClass;

        AsyncLoadHandle = Streamable.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([
            this,
            SoftMesh,
            SoftThrowMontage,
            SoftStrategyClass,
            SoftExplosionEffect,
            SoftDamageAreaClass
        ]()
        {
            if (!IsValid(this)) return;

            // 1. 스태틱 메쉬 설정 (UStaticMeshComponent 캐스팅 또는 전용 메쉬 확인)
            if (SoftMesh.IsValid())
            {
                // AC_WeaponBase의 m_WeaponMesh가 USceneComponent/UMeshComponent이거나 
                // StaticMeshComponent로 다운캐스팅이 필요한 경우 대응
                if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(m_WeaponMesh))
                {
                    StaticMeshComp->SetStaticMesh(SoftMesh.Get());
                }
            }

            // 2. 에셋 캐싱 (IsValid 체크 후 Get()으로 안전하게 할당)
            if (SoftThrowMontage.IsValid())     m_ThrowMontage         = SoftThrowMontage.Get();
            if (SoftStrategyClass.IsValid())    m_ExplodeStrategyClass = SoftStrategyClass.Get();
            if (SoftExplosionEffect.IsValid())  m_ExplosionEffect      = SoftExplosionEffect.Get();
            if (SoftDamageAreaClass.IsValid())  m_FireDamageAreaClass  = SoftDamageAreaClass.Get();

            UC_Util::Print("Throwable Weapon Assets Async Loaded Successfully!", FColor::Green, 5.f);

            // 로딩 완료 후 핸들 정리
            if (AsyncLoadHandle.IsValid())
            {
                AsyncLoadHandle.Reset();
            }
        }));
    }
}

bool AC_ThrowableWeaponBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	PRINT_LOCAL(GetWorld(), "Gun - AttachingToHand", FColor::Red, 10.f);
	
	// 투척류를 장착하는 경우, 투척류 상태 초기화
	ResetThrowableState();

	SetActorHiddenInGame(false);

	m_ProjectileMovement->Deactivate();

	// 예측 경로 제거
	ClearPredictedPath(); 

	// Self init (이 변수들 처리 추후, 던지기 기다리기 처리 시 필요함)
	// bIsCharging       = false;
	// bIsOnThrowProcess = false;

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
	{
		Player->SetHandState(EHandState::WeaponThrowable);
		UpdateAmmoInfoHUDForDrawEnd();
	}
	
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
	CancelThrowAction();

	SetActorHiddenInGame(true);
	m_ProjectileMovement->Deactivate();

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		s_HolsterSocketName
	);
	
	if (bIsAttached)
		m_OwnerPlayer = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	
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

	// 무기 사용자를 저장해둠 (애님 노티파이 이벤트에서 사용하기 위함) -> 수정(동현) : WeaponBase의 m_OwnerPlayer(자기자신의 Slot에 장착한 무기주인)을 사용
	// m_WeaponUser = _WeaponUser;

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

	if (m_OwnerPlayer != _WeaponUser)
		m_OwnerPlayer = _WeaponUser;

	return OnStartCookInput();
}

bool AC_ThrowableWeaponBase::Server_DecreaseCurCount_Validate()
{
	return true;
}

void AC_ThrowableWeaponBase::Server_DecreaseCurCount_Implementation()
{
	if (ItemLinkComp)
	{
		if (FInventoryEntry* SlotEntry = ItemLinkComp->GetItemEntryPtr())
		{
			--SlotEntry->CurCount;
			int32 Idx = ItemLinkComp->GetSlotIndex();
			if (SlotEntry->CurCount <= 0)
			{
				SlotEntry->Clear();
				SlotEntry->SlotIndex = Idx;
			}
			// TODO : 직후에 인벤토리를 업데이트 해주어야 함.

			
			// 서버에서 사용하면 이걸 써야 할 걸?
			
			ItemLinkComp->GetOwningInvenComp()->MarkSlotDirty(Idx);		
			
			ItemLinkComp->GetOwningInvenComp()->OnInventorySlotChanged.Broadcast(Idx, *SlotEntry);
		}
	}
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

	// 투척류 예측 경로 제거
	ClearPredictedPath();
	

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
	// 멀티 환경에서 여기서 터지는 듯?
	
	UAnimInstance* AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;
	
	// 차징 중이면, ReadyLoop 상태로 넘어가고 투척 동작으로 넘어가지 않음
	if (m_bIsCharging)
	{
		m_ThrowableState = EThrowableState::ReadyLoop;

		// 투척류 예측 경로 그리기
		UpdatePredictedPath();

		return;
	}

	// 차징이 끝났으면, 예측 경로 제거
	ClearPredictedPath();

	// 차징이 끝났으면, 투척 동작으로 넘어감
	m_ThrowableState = EThrowableState::Throwing;

	m_OwnerPlayer->PlayAnimMontage(m_ThrowMontage, 1.f, m_ThrowSectionName);
}

void AC_ThrowableWeaponBase::OnThrowThrowable()
{
	if (!m_OwnerPlayer)
		return;

	if (!m_MainCollider || !m_ProjectileMovement)
		return;

	// 투척류 예측 경로 제거
	ClearPredictedPath();

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
}

void AC_ThrowableWeaponBase::OnThrowProcessEnd()
{
	PRINT_LOCAL(GetWorld(), "OnThrowProcessEnd", FColor::MakeRandomColor(), 20.f);
	
	// TODO 
	// 수류탄 던짐
	// EquippedComponent의 CurrentWeapon은 nullptr 또는 다음 수류탄으로 변경
	// 수류탄 개수 감소

	// 2개 남았을 경우까지만 체킹
	
	// TODO : 멀티 환경에 맞게 조정해야 할 수 있음.
	// 투척류는 아이템 갯수가 Ammo라고 보면된다.
	// 투척하면 하나씩 줄여주고 0이 되면 해당 슬롯의 Entry를 비워준다.
	// 이 작업을 서버에서 처리하면 될 듯?
	Server_DecreaseCurCount();
	
	// TODO : 던지고 Count 남아있으면 새로 스폰해주던지, 던질 때 가짜를 던지던지 해야함.

	// int32 Idx = ItemLinkComp->GetSlotIndex();

	// m_OwnerPlayer->GetEquippedComponent()->Server_RequestSpawnEquippedActor(static_cast<int32>(EWeaponSlot::ThrowableWeapon), )
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
	// 폭발 처리는 II_ExplodeStrategy를 상속받은 클래스에서 처리
	if (m_ThrowableState == EThrowableState::Exploded)
		return;

	// 손에 들고 있는 상태에서 폭발할 수 있으므로 여기서도 예측 경로 제거
	ClearPredictedPath();

	const EThrowableState PrevState = m_ThrowableState;

	/// TODO : 폭발 시, 투척류 애님 몽타주가 재생 중이면 Stop 처리
	/// 수류탄을 들고있는 기본 상태보다 아무것도 들고있지 않는 기본 상태로
	if (PrevState != EThrowableState::Thrown && PrevState != EThrowableState::Throwing)
	{
		UAnimInstance* AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();

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
	// 폭발 전략 객체가 없거나, Interface를 구현하지 않은 경우, Actor 제거
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
		if (m_ExplosionEffect)
		{
			// 폭발 이펙트 생성
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld()
				, m_ExplosionEffect
				, GetActorLocation()
				, GetActorRotation()
				, FVector(m_ExplosionEffectScale) 
				, true);	// 재생 종료 후 자동 제거
		}
	}
	
	// 폭발 처리 완료 후, Actor 제거
	SetActorHiddenInGame(true);
	Destroy();
}

// ----------------- 투척 취소 관련 처리 -----------------

void AC_ThrowableWeaponBase::CancelThrowAction()
{
	//// 지금은 사용 안하지만 나중에 투척 캔슬시에 사용

	ClearPredictedPath();

	if (!m_OwnerPlayer)
		return;

	UAnimInstance* AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();

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

	// m_WeaponUser = nullptr; // TODO : 이거 nullptr로 처리함으로써 다른 함수에서 영향을 미치는지 확인해봐야 함

	// 이전 충돌 정보 초기화
	m_HitResult = FHitResult();
}


// ----------------- 투척 관련 처리 -----------------

// 플레이어가 바라보는 방향을 기준으로 투척 방향 반환
FVector AC_ThrowableWeaponBase::GetThrowDirection() const
{
	if (!m_OwnerPlayer)
		return GetActorForwardVector();

	// 플레이어가 바라보는 방향을 기준으로 투척 방향 계산
	FVector ThrowDirection = m_OwnerPlayer->GetActorForwardVector();

	// 마우스 방향을 기준으로 투척 방향 계산
	if (AController* Controller = m_OwnerPlayer->GetController())
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

	if (m_OwnerPlayer)
	{
		// 캐릭터가 바라보는 방향을 기준으로 투척 시작 위치 계산
		const FVector CharacterForward = m_OwnerPlayer->GetActorForwardVector().GetSafeNormal();

		LaunchLocation += CharacterForward * m_LaunchForwardOffset;
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

	if (m_OwnerPlayer)
	{
		// Owner와 충돌하지 않도록 설정
		m_MainCollider->IgnoreActorWhenMoving(m_OwnerPlayer, true);
		m_OwnerPlayer->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
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
	if (OtherActor == m_OwnerPlayer)
		return;

	// 충돌 시 폭발 하도록 설정되어 있지 않으면 무시
	if (!m_bExplodeOnImpact)
		return;

	// 충돌이 발생했는가
	if (Hit.bBlockingHit)
	{
		// 폭발 전에 실제 충돌 정보 저장
		m_HitResult = Hit;
	}

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

void AC_ThrowableWeaponBase::UpdatePredictedPath()
{
	// 이전 호출 시점에 그려진 예측 경로를 제거
	ClearPredictedPath();

	if (!m_OwnerPlayer || !m_PathSpline)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 투척 방향과 투척 시작 위치 가져오기
	const FVector ThrowDirection = GetThrowDirection();
	const FVector LaunchLocation = GetLaunchLocation(ThrowDirection);

	// 투척 속도 가져오기
	const float ThorwSpeed = m_ThrowSpeed > 0.f ? m_ThrowSpeed : 1500.f;
	const FVector LaunchVelocity = ThrowDirection.GetSafeNormal() * ThorwSpeed;


	//----------------- 투척류 예측 경로 계산 옵션 설정 -----------------
	
	FPredictProjectilePathParams PathParams;

	PathParams.StartLocation = LaunchLocation;
	PathParams.LaunchVelocity = LaunchVelocity;
	PathParams.bTraceWithCollision = true;
	if (UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(m_MainCollider))
	{
		PathParams.ProjectileRadius = Capsule->GetScaledCapsuleRadius();
	}
	PathParams.MaxSimTime = m_PredictedPatchMaxTime;
	PathParams.bTraceWithChannel = true;
	PathParams.TraceChannel = m_PredictedPatchTraceChannel;
	PathParams.SimFrequency = m_PredictedPatchSimFrequency;
	PathParams.OverrideGravityZ = m_ProjectileMovement->GetGravityZ();
	PathParams.bTraceComplex = false; 
	PathParams.DrawDebugType = EDrawDebugTrace::None;
	PathParams.ActorsToIgnore.Add(this);
	PathParams.ActorsToIgnore.AddUnique(m_OwnerPlayer);

	// ----------------- 투척류 예측 경로 계산 -----------------

	FPredictProjectilePathResult PathResult;

	
	// 위치/속도/중력 기준으로 투척류의 예상 경로를 계산하고, 충돌 여부를 반환
	const bool bHit = UGameplayStatics::PredictProjectilePath(World, PathParams, PathResult);

	// PathResult.PathData에 계산된 경로가 2개 미만이면 예측 경로를 그릴 수 없으므로 return
	if (PathResult.PathData.Num() < 2)
		return;

	// ----------------- 예상 위치 SplinePoint 로 추가 -----------------
	
	for (const FPredictProjectilePathPointData& PointData : PathResult.PathData)
	{
		// PredictProjectilePath 결과는 월드좌표로 반환되기때문에
		// 카메라·캐릭터 위치에 따라 경로가 크게 어긋날 수 있으므로 World로 추가해야 한대요
		m_PathSpline->AddSplinePoint(PointData.Location, ESplineCoordinateSpace::World, true);
	}

	// 경로 점을 곡선 타입으로 설정
	for (int32 Index = 0; Index < m_PathSpline->GetNumberOfSplinePoints(); ++Index)
	{
		m_PathSpline->SetSplinePointType(Index, ESplinePointType::Curve, false);
	}

	// Spline 갱신
	m_PathSpline->UpdateSpline(); 

	// ----------------- 예측 충돌 지점 표시 -----------------

	if (bHit && m_PredictedEndPoint)
	{
		// 실제 충돌한 위치
		const FVector ImpactPoint = PathResult.HitResult.ImpactPoint;

		// 충돌한 Normal 벡터
		const FVector ImpactNormal = PathResult.HitResult.ImpactNormal;

		// 충돌 지점에서 약간 떨어진 위치에 PredictedEndPoint를 배치하고 보여줌
		m_PredictedEndPoint->SetWorldLocation(ImpactPoint + ImpactNormal * 1.f);
		m_PredictedEndPoint->SetVisibility(true);
	}

	// StaticMesh 지정안되어 있으면 선 생성 X
	if (!m_PredictedPathMesh)
		return;

	// ----------------- SplinePoint 사이마다 Spline Mesh 생성 -----------------

	// SplinePoint 개수 가져오기
	const int32 SplinePointCount = m_PathSpline->GetNumberOfSplinePoints();

	for (int32 Index = 0; Index < SplinePointCount - 1; ++Index)
	{
		FVector StartLocation{};	// 시작점 위치
		FVector StartTangent{};		// 시작점 곡선이 뻗는 방향

		FVector SecondLocation{};	// 끝점 위치
		FVector SecondTangent{};	// 끝점 곡선이 뻗는 방향

		// 현재 구간의 시작점 위치와 곡선 방향 가져오기 
		m_PathSpline->GetLocationAndTangentAtSplinePoint
		(
			Index,
			StartLocation,
			StartTangent,
			ESplineCoordinateSpace::Local // PathSpline 의 자식으로 연결할것이므로 Local
		);

		// 현재 구간의 끝점 위치와 곡선 방향 가져오기
		m_PathSpline->GetLocationAndTangentAtSplinePoint
		(
			Index + 1,
			SecondLocation,
			SecondTangent,
			ESplineCoordinateSpace::Local // PathSpline 의 자식으로 연결할것이므로 Local
		);

		// 두 SplinePoint 사이에 SplineMeshComponent 동적 생성
		USplineMeshComponent* PathMeshComponent = NewObject<USplineMeshComponent>(this);
		PathMeshComponent->SetStaticMesh(m_PredictedPathMesh);	// 메시 설정
		PathMeshComponent->SetStartAndEnd(StartLocation, StartTangent, SecondLocation, SecondTangent); // 시작점과 끝점 설정
		PathMeshComponent->SetMobility(EComponentMobility::Movable); // 조준 방향에 따라 변경되므로 Movable로 설정
		PathMeshComponent->RegisterComponentWithWorld(GetWorld()); // 월드에 등록
		PathMeshComponent->AttachToComponent(m_PathSpline, FAttachmentTransformRules::KeepRelativeTransform); // PathSpline에 자식으로 연결

		// 배열에 저장
		m_PredictedPathMeshes.Add(PathMeshComponent);
	}
}

void AC_ThrowableWeaponBase::ClearPredictedPath()
{
	// 충돌 위치 표시 제거
	if (m_PredictedEndPoint)
	{
		m_PredictedEndPoint->SetVisibility(false);
	}

	// 모든 SplinePoint 제거
	for (USplineMeshComponent* PathMeshComponent : m_PredictedPathMeshes)
	{
		if (!IsValid(PathMeshComponent))
			continue;

		PathMeshComponent->DestroyComponent();
	}

	// 배열 초기화
	m_PredictedPathMeshes.Empty();

	// 기존 Spline 제거
	if (m_PathSpline)
	{
		// SplinePoint 제거할때마다 갱신하지 않도록 false로 설정
		m_PathSpline->ClearSplinePoints(false);
		// 점 제거 끝난 후 Spline 한번만 갱신
		m_PathSpline->UpdateSpline();
	}
}

void AC_ThrowableWeaponBase::UpdateAmmoInfoHUDForDrawEnd()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;
	UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, EFireMode::Single, 1, 1);
}

void AC_ThrowableWeaponBase::SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo)
{
	_AmmoUIInfo.Visible            = true;
	_AmmoUIInfo.FireMode           = EFireMode::Single;
	_AmmoUIInfo.MagazineAmmo       = 1;
	_AmmoUIInfo.LeftAmmoTotalCount = 1;
	
}
