#include "Item/PickUp/C_ItemPickUp.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

AC_ItemPickUp::AC_ItemPickUp()
{
	PrimaryActorTick.bCanEverTick = false;

    // 구체 컴포넌트 생성 및 루트 설정
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(80.0f); // 줍는 범위 설정

    // 물리 및 충돌 프로필 설정 (플레이어와 겹침 감지가 가능하도록)
    //CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // 스태틱 메시 컴포넌트 생성 및 첨부
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

    // 메시는 순수 비주얼용이므로 물리 연산 및 캐릭터 밀어내기를 끕니다.
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 변수 초기화
    bPickup = false;
}

void AC_ItemPickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

void AC_ItemPickUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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

