// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Storage/C_Storage.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/SphereComponent.h"
#include "GameMode/C_UIManager.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "Utility/C_Util.h"

// Sets default values
AC_Storage::AC_Storage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	
	SphereComp =  CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComp"));
	SphereComp->SetSphereRadius(100.f);
	SphereComp->SetupAttachment(RootComponent);
	
	InvenComp = CreateDefaultSubobject<UC_InvenComponent>(TEXT("InvenComponent"));
}

// Called when the game starts or when spawned
void AC_Storage::BeginPlay()
{
	Super::BeginPlay();
	
	if (SphereComp)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AC_Storage::OnOverlapBegin);
		SphereComp->OnComponentEndOverlap.AddDynamic(this, &AC_Storage::OnOverlapEnd);
		
	}
	
}

void AC_Storage::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	
	if (!Player) return;
	
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	
	if (!PC) return;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return;
	
	UIManager->GetInventoryWidget()->GetStorageGridWidget()->SetInvenComponent(InvenComp);
	UIManager->GetInventoryWidget()->GetStorageGridWidget()->RefreshAllSlots(InvenComp->GetInventoryItems());
	UC_Util::Print("Storage Overlap!");
	
	if (InvenComp)
	{
		UC_Util::Print("Storage Inven Items Num: " + FString::FromInt(InvenComp->GetInventoryItems().Num()));
	}
	else
	{
		UC_Util::Print("Storage InvenComponent is NULL!");
	}
}

void AC_Storage::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	
	if (!Player) return;
	
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	
	if (!PC) return;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return;
	
	UIManager->GetInventoryWidget()->GetStorageGridWidget()->SetInvenComponent(nullptr);
	UIManager->GetInventoryWidget()->GetStorageGridWidget()->SetVisibility(ESlateVisibility::Collapsed);
}

// Called every frame
void AC_Storage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

