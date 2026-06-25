#include "Item/PickUp/C_ItemPickUp.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
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

void AC_ItemPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

