#include "Item/PickUp/C_ItemPickUp.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Utility/C_Util.h"

AC_ItemPickUp::AC_ItemPickUp()
{
	PrimaryActorTick.bCanEverTick = false;

    // 물리 구체를 생성하고 루트 컴포넌트로 설정합니다.
    PhysicsSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicsSphere"));
    RootComponent = PhysicsSphere;
    PhysicsSphere->SetSphereRadius(15.0f); // 아이템 자체 크기에 맞게 작게 설정 (땅에 구르는 용도)
    
    // 물리가 가능하도록 셋팅 (이것이 던지기의 핵심!)
    PhysicsSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicsSphere->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    PhysicsSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    // 플레이어 캡슐과는 겹치게(Overlap) 해서 플레이어를 밀어내지 않게 설정하는 것이 좋습니다.
    PhysicsSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    
    PhysicsSphere->SetLinearDamping(1.5f);
    
    PhysicsSphere->SetAngularDamping(2.0f);
    // 기본적으로 물리를 켜둡니다. (Manager에서 명시적으로 켜도 됨)
    PhysicsSphere->SetSimulatePhysics(true);

    // 태틱 메시 컴포넌트 생성 및 첨부
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
    PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    // 변수 초기화
    bPickup = false;
}

void AC_ItemPickUp::BeginPlay()
{
	Super::BeginPlay();
    
    if (PickupSphere)
    {
        PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AC_ItemPickUp::OnOverlapBegin);
    }
}

void AC_ItemPickUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. 자기 자신과의 충돌 방지 및 유효성 검사
    if (!OtherActor || OtherActor == this) return;

    // 이미 누군가 주워가는 중이라면 중복 처리 방지
    if (bPickup) return;

    // 2. 충돌한 대상(플레이어 등)에게 인벤토리 컴포넌트가 있는지 확인
    AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);

    if (!IsValid(Player)) return;

    UC_InvenComponent* PlayerInvenComp = Player->GetInvenComponent();

    UC_Util::Print("OverlapBegin");

    if (PlayerInvenComp)
    {
        // 중복 진입 방지 플래그 On
        bPickup = true;

        // 3. 인벤토리에 아이템 추가 시도
        bool bIsAdded = PlayerInvenComp->AddItem(ItemData);

        if (bIsAdded)
        {
            // 4. 아이템 획득에 성공했다면 필드의 아이템 액터 삭제
            Destroy();
            UC_Util::Print("Success Add Item to Inventory!");
        }
        else
        {
            // 인벤토리가 가득 찼거나 추가에 실패한 경우 플래그 원복
            bPickup = false;
            UC_Util::Print("Failed Add Item to Inventory!");

            // 필요하다면 화면에 "인벤토리가 가득 찼습니다" 메시지 출력
        }
    }
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

void AC_ItemPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

