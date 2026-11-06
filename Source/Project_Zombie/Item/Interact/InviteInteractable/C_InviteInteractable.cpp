// Fill out your copyright notice in the Description page of Project Settings.


#include "C_InviteInteractable.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/FaceToLocalPlayerComponent/C_FaceToLocalPlayerComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MenuUI/C_MenuWidget.h"
#include "Utility/C_Util.h"


AC_InviteInteractable::AC_InviteInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	m_MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(m_MeshComponent);
	
	m_BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	m_BoxComponent->SetupAttachment(m_MeshComponent);
	
	m_FaceToLocalPlayerComponent = CreateDefaultSubobject<UC_FaceToLocalPlayerComponent>(TEXT("FaceToLocalPlayerComponent"));
	m_ExplanationWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ExplanationWidgetComponent"));
	m_ExplanationWidgetComponent->SetupAttachment(m_MeshComponent);
}

void AC_InviteInteractable::BeginPlay()
{
	Super::BeginPlay();

	/*if (!m_ExplanationWidgetComponent || m_ExplanationWidgetComponent->GetWidget())
	{
		UC_Util::Print("[AC_InviteInteractable::BeginPlay] : Invalid ExplanationWidget", FColor::Red, 10.f);
		return;
	}*/
	
	// TODO : Local player가 방장인 경우에 한해서 활성화 처리 해줄 것 (이 클래스 내부에서 자체적으로)
	
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		UC_GameLevelManager* GameLevelManager = LEVEL_MANAGER;
		
		if (!GameLevelManager) return;
		if (!GameLevelManager->GetLocalPlayer()) return;

		// 방장인 경우에 한해 Invite 기능 활성화 및 Explanation 내용 넣기
		if (GameLevelManager->GetLocalPlayer()->HasAuthority())
		{
			m_ExplanationWidgetComponent->SetHiddenInGame(false);
			InitExplanationWidgetTextField(); // 글자 Init 처리 (Blueprint로 해야 함, ExplanationWidget 부모 클이 UUserWidget임)
			m_FaceToLocalPlayerComponent->RegisterSceneComToRotate(m_ExplanationWidgetComponent);
			
			m_BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AC_InviteInteractable::BeginOverlap);
			m_BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AC_InviteInteractable::EndOverlap);
		}
		else
		{
			m_ExplanationWidgetComponent->SetHiddenInGame(true); // 방장이 아닌 경우, Invite 기능 활성화 x (빈 깡통 Actor 처리)
			m_BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
				
		GetWorldTimerManager().ClearTimer(m_InitTimerHandle);
	});
	
	GetWorldTimerManager().SetTimer(m_InitTimerHandle, TimerDelegate, 0.3f, false);
}

void AC_InviteInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_InviteInteractable::BeginOverlap
(
	UPrimitiveComponent* OverlappedComp,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex,
	bool				 bFromSweep,
	const FHitResult&	 SweepResult
)
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	if (!Player->IsLocallyControlled()) return;
	if (!Player || !Player->HasAuthority()) return; // 오로지 방장 플레이어만 Interaction Valid

	Player->SetEnteredInviteInteractable(this);
	
	ToggleOutlineEffect(true);
	
	AC_UIManager* UIManager = UI_MANAGER(GetWorld()); 
	if (!UIManager) return;
	if (!UIManager->GetMenuWidget()) return;
	
	// Menu를 이미 보고 있는 중이라면 따로 처리 x (Menu 창에 해당 중복된 기능이 이미 있음)
	if (UIManager->GetMenuWidget()->GetVisibility() == ESlateVisibility::Visible) return;
	
	// Menu를 보고 있지 않은 중이라면, FriendList 활성화 (Interaction 키 처리 없이 바로 해당 창 보여주기 처리)
	if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
		MainHUD->ToggleFriendListWidget(true);
}

void AC_InviteInteractable::EndOverlap
(
	UPrimitiveComponent* OverlappedComp,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex
)
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	if (!Player->IsLocallyControlled()) return;
	if (!Player || !Player->HasAuthority()) return; // 오로지 방장 플레이어만 Interaction Valid

	Player->SetEnteredInviteInteractable(nullptr);
	
	ToggleOutlineEffect(false);
	
	// 친구창 Visible인 경우에 한해서만 Visible off
	if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
	{
		if (MainHUD->GetFriendListWidget()->GetVisibility() == ESlateVisibility::Visible)
			MainHUD->ToggleFriendListWidget(false);
	}
}

void AC_InviteInteractable::ToggleOutlineEffect(bool _Enable)
{
	if (m_MeshComponent) m_MeshComponent->SetOverlayMaterial(_Enable ? m_OutlineMaterial : nullptr);
}
