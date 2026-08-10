// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Storage/C_Storage.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/SphereComponent.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "Utility/C_Util.h"

// Sets default values
AC_Storage::AC_Storage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SetReplicates(true);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	
	
	SphereComp =  CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComp"));
	SphereComp->SetSphereRadius(100.f);
	SphereComp->SetupAttachment(MeshComp);
	
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
	
	if (HasAuthority() && InvenComp)
	{
		// 인벤토리 매니저 서브시스템 가져오기
		if (UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>())
		{
			// 인벤토리의 현재 아이템 총 개수만큼 루프
			const TArray<FInventoryEntry>& Items = InvenComp->GetInventoryItems();
			for (int32 i = 0; i < Items.Num(); ++i)
			{
				// 빈 슬롯이거나 이름이 없다면 패스
				if (Items[i].ItemRowName.IsNone()) continue;

				// 이미 유효한 CustomData를 가지고 있다면 패스
				if (Items[i].CustomData.IsValid()) continue;

				// 인벤토리 내부 원본 데이터의 포인터 주소를 긁어옵니다.
				FInventoryEntry* RawItemPtr = InvenComp->GetSlotDataPtr(i);
				if (RawItemPtr)
				{
					// ItemManager에서 아이템 테이블 기본 데이터를 조회
					if (const FItemData* BaseData = ItemManager->GetItemData<FItemData>(EItemTableType::General, RawItemPtr->ItemRowName))
					{
						
						//if (static_cast<uint8>(BaseData->ItemType) <= static_cast<uint8>(EItemType::CONSUMABLE)) continue;
						
						// ❌ 누락되었던 기본 CustomData를 인스턴스 구조체로 생성하여 주입!
						RawItemPtr->CustomData = FInstancedStruct::Make(BaseData->CustomData);

						// 🔄 Fast Array Container 상태를 더티 마킹하여 클라이언트에 강제 복제 요청
						InvenComp->MarkSlotDirty(i);

						UE_LOG(LogTemp, Log, TEXT("📦 [Storage Init] 슬롯 [%d]의 '%s' 아이템 CustomData 주입 및 동기화 완료!"), 
							i, *RawItemPtr->ItemRowName.ToString());
					}
				}
			}
		}
	}
}

void AC_Storage::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
		UC_Util::Print("Server Executed");
	else
		UC_Util::Print("Client Executed");
	
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	
	if (!Player)
	{
		UC_Util::Print("Can't find player!");
		return;
	}
	
	if (!Player->IsLocallyControlled()) return;
	
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	
	if (!PC)
	{
		UC_Util::Print("Can't find player controller!");
		return;
	}
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager)
	{
		UC_Util::Print("Can't find UIManager!");
		return;
	}
	UC_InventoryWidget* InventoryWidget = UIManager->GetInventoryWidget();
	
	InventoryWidget->GetStorageGridWidget()->SetInvenComponent(InvenComp);
	
	if (InventoryWidget->GetVisibility() == ESlateVisibility::Visible)
		InventoryWidget->GetStorageGridWidget()->SetVisibility(ESlateVisibility::Visible);
	
	//UIManager->GetInventoryWidget()->GetStorageGridWidget()->RefreshAllSlots(InvenComp->GetInventoryItems());
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

