#include "Item/PickUp/C_ItemPickUp.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_PingSystemComponent.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Net/UnrealNetwork.h"
#include "Utility/C_Util.h"

AC_ItemPickUp::AC_ItemPickUp()
{
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);
    
    // 물리 구체를 생성하고 루트 컴포넌트로 설정합니다.
    PhysicsSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicsSphere"));
    SetRootComponent(PhysicsSphere);   
    
    PhysicsSphere->SetSphereRadius(15.0f); // 아이템 자체 크기에 맞게 작게 설정 (땅에 구르는 용도)
    
    // 물리가 가능하도록 셋팅 (이것이 던지기의 핵심!)
    PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicsSphere->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    PhysicsSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    // 플레이어 캡슐과는 겹치게(Overlap) 해서 플레이어를 밀어내지 않게 설정하는 것이 좋습니다.
    PhysicsSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    PhysicsSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    
    PhysicsSphere->SetLinearDamping(1.5f);
    
    PhysicsSphere->SetAngularDamping(2.0f);
    // 기본적으로 물리를 켜둡니다. (Manager에서 명시적으로 켜도 됨)
    PhysicsSphere->SetSimulatePhysics(true);

    // 물리 및 충돌 프로필 설정 (플레이어와 겹침 감지가 가능하도록)
    //CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // 스태틱 메시 컴포넌트 생성 및 첨부

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 비주얼용 유지

    // 줍는 범위 감지용 구체를 자식으로 붙입니다.
    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    PickupSphere->SetupAttachment(RootComponent);
    PickupSphere->SetSphereRadius(50.0f); // 줍는 범위는 크게 유지
    PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // 플레이어만 감지하도록 셋팅
    PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    //PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    // 변수 초기화
    bPickup = false;
    
    //bReplicates = true;
    //SetReplicateMovement(true);
}

void AC_ItemPickUp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    

    
    // endplay호출 시 풀로 돌려도 되는가? 아니면 그냥 삭제되게 두어야 하는가?
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UC_ItemManager* ItemMgr = GI->GetSubsystem<UC_ItemManager>())
        {
            ItemMgr->ReturnToPool(this);
        }
    }
    
    if (UWorld* World = GetWorld())
    {
        GetWorldTimerManager().ClearTimer(DespawnTimerHandle);
    }
}

void AC_ItemPickUp::BeginPlay()
{
	Super::BeginPlay();
    
    if (PickupSphere)
    {
        PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AC_ItemPickUp::OnOverlapBegin);
    }
    
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            PickupDelayTimerHandle, 
            this, 
            &AC_ItemPickUp::EnablePickupOverlap, 
            DELAYTIME, 
            false
        );
    }
}

void AC_ItemPickUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 물리 이벤트는 서버에서만 처리
    if (!HasAuthority()) return;
    
    if (bPickup) return;
    
    AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
    
    if (!Player) return;
    
    // 서버 함수 호출
    Server_RequestPickup(Player);
}

void AC_ItemPickUp::OnMeshLoadCompleted(TSoftObjectPtr<UStaticMesh> LoadedSoftMesh)
{
    // 로드가 완료되면 이 함수가 자동으로 실행됩니다.
    if (LoadedSoftMesh.IsValid() && MeshComp)
    {
        MeshComp->SetStaticMesh(LoadedSoftMesh.Get());
    }

    // 사용이 끝난 핸들은 초기화해 줍니다.
    AssetLoadHandle.Reset();
}


void AC_ItemPickUp::SetPickupMeshAsync(TSoftObjectPtr<UStaticMesh> InSoftMesh)
{
    if (InSoftMesh.IsNull()) return;

    // 만약 이미 메모리에 로드되어 있는 상태라면, 굳이 대기하지 않고 즉시 적용합니다.
    if (InSoftMesh.IsValid())
    {
        if (MeshComp) MeshComp->SetStaticMesh(InSoftMesh.Get());
        return;
    }

    // 에셋 매니저를 통해 비동기 로드를 요청합니다.
    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    // 콜백 함수에 어떤 에셋을 로드 중이었는지 인자(InSoftMesh)를 묶어서 전달(BindUFunction)합니다.
    AssetLoadHandle = StreamableManager.RequestAsyncLoad(
        InSoftMesh.ToSoftObjectPath(),
        FStreamableDelegate::CreateUFunction(this, FName("OnMeshLoadCompleted"), InSoftMesh)
    );
}

void AC_ItemPickUp::EnablePickupOverlap()
{
    if (PickupSphere)
    {
        UC_Util::Print("EnablePickUpOverlap");
        // 이제 플레이어(Pawn)를 감지하도록 오버랩 설정 변경
        PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        // (선택 사항) 만약 스폰 시점에 이미 플레이어가 범위 안에 겹쳐있었다면 
        // 자동으로 감지하지 못할 수 있으므로, 강제로 주변 오버랩을 갱신해 줍니다.
        PickupSphere->UpdateOverlaps();
    }
}

void AC_ItemPickUp::ActivateItem(const FInventoryEntry& InEntry, const FVector& SpawnLocation)
{
    // 1. 위치 이동 (TeleportPhysics 옵션 필수! - 이전 -10000 위치에서의 물리 연산 충격 방지)
    SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    SetActorTickEnabled(false);

    bPickup = false;
    ItemEntry = InEntry;

    // 2. 서버 전용 로직 (물리 제어, 타이머, 수거 처리)
    if (HasAuthority())
    {
        if (PhysicsSphere)
        {
            PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PhysicsSphere->SetSimulatePhysics(true);
            
            // 물리 속도 및 회전 속도 초기화
            PhysicsSphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
            PhysicsSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            
            // 물리가 멈췄는지 체크하는 타이머는 서버에서만 관리
            GetWorldTimerManager().SetTimer
            (
                m_SimulatePhysicsStoppedCheckTimer,
                this,
                &AC_ItemPickUp::CheckPhysicsStopped,
                0.5f,
                true
            );
        }

        // 3. 줍기 가능 지연 타이머 (서버 전용)
        GetWorldTimerManager().SetTimer(
            PickupDelayTimerHandle, 
            this, 
            &AC_ItemPickUp::EnablePickupOverlap, 
            DELAYTIME, 
            false
        );

        // 4. 시한부 자동 수거 타이머 (서버 전용)
        StartDespawnTimer(DefaultLifeTime);
    }

    /*// 1. 위치 및 충돌/시각 켜기
    SetActorLocation(SpawnLocation);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    SetActorTickEnabled(false); // Tick을 안 쓴다면 꺼둠

    bPickup = false;
    ItemEntry = InEntry;

    // 2. 물리 구체 리셋 및 초기화
    if (PhysicsSphere)
    {
        PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PhysicsSphere->SetSimulatePhysics(true);
        //PhysicsSphere->SetAllPhysicsVelocity(FVector::ZeroVector);
        //PhysicsSphere->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        PhysicsSphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PhysicsSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        
        // SimulatePhysics에 의한 움직임이 멈췄을 때, SimulatePhysics 끄기 및 기타 처리를 위함
        GetWorld()->GetTimerManager().SetTimer
        (
            m_SimulatePhysicsStoppedCheckTimer,
            this,
            &AC_ItemPickUp::CheckPhysicsStopped,
            0.5f,
            true
        );
    }

    // 3. 줍기 오버랩 구체 초기화 (기존 DELAYTIME 후 켜지던 타이머 재가동)
    //if (PickupSphere)
    //{
    //    PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    //}

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            PickupDelayTimerHandle, 
            this, 
            &AC_ItemPickUp::EnablePickupOverlap, 
            DELAYTIME, 
            false
        );

        // 4. 시한부 자동 수거 타이머 시작 (서버에서만)
        StartDespawnTimer(DefaultLifeTime);
    }*/
}

void AC_ItemPickUp::DeactivateItem()
{
    // [보안] 서버에서만 실행되어야 하는 정리 작업들
    if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(PickupDelayTimerHandle);
        GetWorldTimerManager().ClearTimer(DespawnTimerHandle);
        GetWorldTimerManager().ClearTimer(m_SimulatePhysicsStoppedCheckTimer);

        // StolenPlayerPingSystemComponent 정리
        if (m_StolenPlayerPingSystemComponent && m_StolenPlayerPingSystemComponent->GetLastInstigator() == this)
        {
            m_StolenPlayerPingSystemComponent->Multicast_MustHidePingAll();
        }
    }

    // 1. 비동기 로드 핸들 즉시 취소 (메모리 릭 및 지연 로드 방지)
    if (AssetLoadHandle.IsValid() && AssetLoadHandle->IsActive())
    {
        AssetLoadHandle->CancelHandle();
    }
    AssetLoadHandle.Reset();

    // 2. 물리 및 충돌 비활성화
    if (PhysicsSphere)
    {
        PhysicsSphere->SetSimulatePhysics(false);
        PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (PickupSphere)
    {
        PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    SetActorEnableCollision(false);

    // 3. 시각 및 렌더링 끄기 (서버에서 설정하면 클라이언트로 복제됨)
    SetActorHiddenInGame(true);
    SetActorLocation(FVector(0.f, 0.f, -10000.f)); // 안전지대로 대피

    // 4. 로컬 비주얼 및 데이터 초기화
    if (MeshComp)
    {
        // Multicast 대신 로컬에서 직접 Outline 끄기
        // (필요 시 SetRenderCustomDepth(false) 등 로컬 처리)
        MeshComp->SetRenderCustomDepth(false); 
        MeshComp->SetStaticMesh(nullptr);
    }

    ItemEntry = FInventoryEntry();
    MeshRef = nullptr;
    /*if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(PickupDelayTimerHandle);
        GetWorldTimerManager().ClearTimer(DespawnTimerHandle);
    }

    // 1. 물리 및 충돌 비활성화
    if (PhysicsSphere)
    {
        PhysicsSphere->SetSimulatePhysics(false);
        PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (PickupSphere)
    {
        PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 2. 시각 및 렌더링 끄기
    SetActorLocation(FVector(0.f, 0.f, -10000.f));
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // 3. 비동기 핸들 취소
    if (AssetLoadHandle.IsValid() && AssetLoadHandle->IsActive())
    {
        AssetLoadHandle->CancelHandle();
    }
    AssetLoadHandle.Reset();

    // 데이터 초기화
    ItemEntry = FInventoryEntry();
    MeshRef = nullptr;
    //bPickup = false;
    if (MeshComp)
    {
        Multicast_ToggleOutline(false); // 외곽선 비활성화
        MeshComp->SetStaticMesh(nullptr);
    }
    
    // SimulatePhysics 움직임 체킹 타이머 비활성화
    GetWorld()->GetTimerManager().ClearTimer(m_SimulatePhysicsStoppedCheckTimer);
    
    // StolenPlayerPingSystemComponent가 존재했었다면 clear 및 해당 핑 비활성화(필요하다면)
    if (m_StolenPlayerPingSystemComponent)
    {
        if (m_StolenPlayerPingSystemComponent->GetLastInstigator() == this)
            m_StolenPlayerPingSystemComponent->Multicast_MustHidePingAll();
    }*/
}

void AC_ItemPickUp::StartDespawnTimer(float InLifeTime)
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().SetTimer(
        DespawnTimerHandle,
        [this]()
        {
            if (this)
            {
                // 시간 다 되면 알아서 풀로 반환
               if (UGameInstance* GI = GetGameInstance())
               {
                   if (UC_ItemManager* ItemMgr = GI->GetSubsystem<UC_ItemManager>())
                   {
                       ItemMgr->ReturnToPool(this);
                   }
               }
            }
        },
        InLifeTime,
        false
    );
}

void AC_ItemPickUp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AC_ItemPickUp, bPickup);
    DOREPLIFETIME(AC_ItemPickUp, MeshRef);
    DOREPLIFETIME(AC_ItemPickUp, ItemEntry);
}

void AC_ItemPickUp::OnRep_MeshRef()
{
    SetPickupMeshAsync(MeshRef);
}

void AC_ItemPickUp::OnRep_ItemEntry()
{
    
}

void AC_ItemPickUp::CheckPhysicsStopped()
{
    // 아직 SimulatePhysics 처리에 의해 움직이는 중
    if (PhysicsSphere->IsAnyRigidBodyAwake()) return;
    
    /* 움직임이 멈춤 */
    
    GetWorld()->GetTimerManager().ClearTimer(m_SimulatePhysicsStoppedCheckTimer);
    
    // PhysicsSphere SimulatePhysics 비활성화 처리
    PhysicsSphere->SetSimulatePhysics(false);

    // 만약 이 ItemPickUp이 StolenWeapon이 Drop되었을 때의 Weapon인 경우
    if (m_StolenPlayerPingSystemComponent)
    {
        // 해당 지점에 이전 주인의 Ping 활성화
        m_StolenPlayerPingSystemComponent->Multicast_MustSpawnAll
        (
            GetActorLocation(),
            EGamePingType::GunBaseMarker,
            EPingShapeType::FullPing,
            this
        );
        
        // 외곽선 활성화
        if (MeshComp) Multicast_ToggleOutline(true);
    }
}

void AC_ItemPickUp::Multicast_ToggleOutline_Implementation(bool _Visible)
{
    if (!MeshComp) return;

    MeshComp->SetRenderCustomDepth(true);
    MeshComp->SetCustomDepthStencilValue(_Visible ? 1 : 0);
}

void AC_ItemPickUp::Server_RequestPickup_Implementation(AC_BasicPlayer* Player)
{
    if (bPickup) return;
    
    UC_InvenComponent* PlayerInvenComp = Player->GetInvenComponent();
    if (!PlayerInvenComp) return;
    
    int32 LeftoverCount = PlayerInvenComp->AddItem(ItemEntry);

    if (LeftoverCount <= 0)
    {
        bPickup = true; 
        
        // --- [수정] Destroy() 대신 ItemManager를 통해 풀로 수거 ---
        if (UGameInstance* GI = GetGameInstance())
        {
            if (UC_ItemManager* ItemMgr = GI->GetSubsystem<UC_ItemManager>())
            {
                ItemMgr->ReturnToPool(this);
                return;
            }
        }
        Destroy(); // 예외 처리용 Fallback
    }
    else if (LeftoverCount < ItemEntry.CurCount)
    {
        ItemEntry.CurCount = LeftoverCount;
        OnRep_ItemEntry();
    }
}

void AC_ItemPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

