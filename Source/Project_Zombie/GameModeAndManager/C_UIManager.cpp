// Fill out your copyright notice in the Description page of Project Settings.


#include "C_UIManager.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "Item/Storage/C_Storage.h"
#include "Kismet/GameplayStatics.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"
#include "UI/MainHUD/InteractionWidget/C_InteractionWidget.h"
#include "UI/MainHUD/GameOverWidget/C_GameOverWidget.h"
#include "UI/MenuUI/C_MenuWidget.h"
#include "UI/SkinUI/C_SkinWidget.h"
#include "Utility/C_Util.h"

AC_UIManager* AC_UIManager::Get(const UWorld* _World)
{
	if (!_World) return nullptr;
		
	// 클라이언트/리슨서버 호스트 화면의 로컬 PlayerController(0번)를 구합니다.
	APlayerController* PC = _World->GetFirstPlayerController();
		
	if (PC && PC->IsLocalController()) return Cast<AC_UIManager>(PC->GetHUD());
	return nullptr;
}

UC_GameMainHUD* AC_UIManager::GetMainHUD(const UWorld* _World)
{
	if (!_World) return nullptr;
	
	AC_UIManager* UIManager = Get(_World);
	if (!UIManager) return nullptr;
	
	return UIManager->m_MainHUDWidget;
}

void AC_UIManager::BeginPlay()
{
	Super::BeginPlay();

	if (!m_MainHUDClass)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : MainHUDClass Subclass nullptr", FColor::Red, 5.f);
		return;
	}

	// 이거 명확한 HUD(UUserWidget) 상위 부모 클래스가 있다하면 해당 Type으로 Casting 시도할 것
	m_MainHUDWidget = Cast<UC_GameMainHUD>(CreateWidget(GetOwningPlayerController(), m_MainHUDClass));
	
	if (!m_MainHUDWidget)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : MainHUDWidget creation failed", FColor::Red, 5.f);
		return;
	}

	m_MainHUDWidget->AddToViewport();
	
	m_MouseCursorNeededWidgets.Add(EMouseCursorNeededWidgetType::InviteFriendWidget, m_MainHUDWidget->GetFriendListWidget());
	m_MouseCursorNeededWidgets.Add(EMouseCursorNeededWidgetType::GameOverWidget, m_MainHUDWidget->GetGameOverWidget());
	
	// InventoryWidget 불러오기.
	if (!m_InventoryWidgetClass)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : InventoryWidget Subclass nullptr", FColor::Red, 5.f);
		return;
	}

	m_InventoryWidget = Cast<UC_InventoryWidget>(CreateWidget(GetOwningPlayerController(), m_InventoryWidgetClass));

	if (!m_InventoryWidget)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : InventoryWidget creation failed", FColor::Red, 5.f);
		return;
	}
	//m_InventoryWidget->GetPlayerGridWidget()->InitializeGrid(nullptr);
	//m_InventoryWidget->GetStorageGridWidget()->InitializeGrid(nullptr);
	
	//m_InventoryWidget->SetOwningPlayer()
	
	m_InventoryWidget->AddToViewport();
	m_InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	m_MouseCursorNeededWidgets.Add(EMouseCursorNeededWidgetType::InvenWidget, m_InventoryWidget);

	// MenuWidget 불러오기.
	if (!m_MenuWidgetClass)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : MenuWidgetClass Subclass nullptr", FColor::Red, 5.f);
		return;
	}

	m_MenuWidget = Cast<UC_MenuWidget>(CreateWidget(GetOwningPlayerController(), m_MenuWidgetClass));

	if (!m_MenuWidget)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : InventoryWidget creation failed", FColor::Red, 5.f);
		return;
	}

	m_MenuWidget->AddToViewport();
	m_MenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	m_MouseCursorNeededWidgets.Add(EMouseCursorNeededWidgetType::MenuWidget, m_MenuWidget);

	if (!m_SkinWidgetClass)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : SkinWidgetClass Subclass nullptr", FColor::Red, 5.f);
		return;
	}

	m_SkinWidget = Cast<UC_SkinWidget>(CreateWidget(GetOwningPlayerController(), m_SkinWidgetClass));

	if (!m_SkinWidget)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : SkinWidget creation failed", FColor::Red, 5.f);
		return;
	}

	m_SkinWidget->AddToViewport();
	m_SkinWidget->SetVisibility(ESlateVisibility::Collapsed);
	m_MouseCursorNeededWidgets.Add(EMouseCursorNeededWidgetType::SkinWidget, m_SkinWidget);
}

void AC_UIManager::DrawHUD()
{
	Super::DrawHUD();
	
#if !UE_BUILD_SHIPPING

	// Canvas나 World가 없으면 그리지 않음
	if (!GetWorld() || !Canvas) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
    
	// 기본 설정값
	const float MarginRight = 30.f; // 오른쪽 끝과의 여백 (px)
	float StartY            = 30.f; // 위쪽 끝과의 여백 (px)
	const float LinePadding = 5.f; // 줄 사이의 간격 (px)

	// 오래된 메시지 삭제 및 그리기
	for (int32 i = m_DebugMessages.Num() - 1; i >= 0; --i)
	{
		if (CurrentTime > m_DebugMessages[i].ExpireTime)
		{
			m_DebugMessages.RemoveAt(i);
			continue;
		}

		float TextWidth  = 0.0f;
		float TextHeight = 0.0f;

		// 1. 현재 폰트 기준, 이 텍스트의 실제 가로/세로 길이를 계산합니다.
		GetTextSize(m_DebugMessages[i].Text, TextWidth, TextHeight);

		// 2. [전체 화면 너비] - [글자 너비] - [오른쪽 여백] = 오른쪽 정렬 X 좌표
		float StartX = Canvas->SizeX - TextWidth - MarginRight;

		// 3. 우측 상단 위치에 텍스트 그리기
		DrawText(m_DebugMessages[i].Text, m_DebugMessages[i].Color, StartX, StartY);

		// 4. 다음 줄 메시지는 글자 높이 + 패딩만큼 아래로 내려서 그리기
		StartY += (TextHeight + LinePadding);
	}
#endif
}

void AC_UIManager::PrintLocalDebugMessage(const FString& _Message, const FColor& _Color, float _Duration)
{
	const float ExpireTime = GetWorld()->GetTimeSeconds() + _Duration;
	m_DebugMessages.Add({ _Message, _Color, ExpireTime });
}

bool AC_UIManager::GetMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType _WidgetType) const
{
	UUserWidget* const* pUserWidget = m_MouseCursorNeededWidgets.Find(_WidgetType);
	
	if (!pUserWidget)
	{
		UC_Util::Print("[AC_UIManager::GetMouseInputNeededWidgetVisibility] : Not containing received WidgetType", FColor::Red, 10.f);
		return false;
	}
	
	return (*pUserWidget)->GetVisibility() == ESlateVisibility::Visible;
}

void AC_UIManager::ToggleMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType _WidgetToToggle, bool _Visible)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UC_Util::Print("[AC_UIManager::ToggleMouseInputNeededWidgetVisibility] : Failed to get PlayerController", FColor::Red, 10.f);
		return;
	}
	
	switch (_WidgetToToggle)
	{
	case EMouseCursorNeededWidgetType::InvenWidget:
	{
		if (_Visible) // 열기
		{
			m_InventoryWidget->SetVisibility(ESlateVisibility::Visible);
			
			// 만약 Entered 한 Storage Actor가 있는 경우, OutlineMat과 Tab instruction 비활성화
			if (m_EnteredStorage)
			{
				m_EnteredStorage->ToggleOutlineEffect(false);
				m_MainHUDWidget->GetInteractionWidget()->ToggleTabInteractionInstruction(false);
			}
		}
		else // 닫기
		{
			if (m_InventoryWidget->GetDivideItemWidget()->GetVisibility() == ESlateVisibility::Visible) return;
		
			// 열려있으면 닫기
			m_InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

			// 만약 Entered 한 Storage Actor가 있는 경우, OutlineMat과 Tab instruction 다시 켜주기
			if (m_EnteredStorage)
			{
				m_EnteredStorage->ToggleOutlineEffect(true);
				m_MainHUDWidget->GetInteractionWidget()->ToggleTabInteractionInstruction(true);
			}
		}
		break;
	}
	case EMouseCursorNeededWidgetType::MenuWidget:
	{
		if (_Visible)
		{
			// 열기
			m_MenuWidget->SetVisibility(ESlateVisibility::Visible);

			// Invite Interactable과 교신 중인 방장이라면, Friend List 창 끄기
			if (m_EnteredInviteInteractable) m_MainHUDWidget->ToggleFriendListWidget(false);
		}
		else
		{
			// 닫기
			m_MenuWidget->SetVisibility(ESlateVisibility::Collapsed);

			// Invite Interactable과 교신 중인 방장이라면, Friend List 창만 띄워주기
			if (m_EnteredInviteInteractable) m_MainHUDWidget->ToggleFriendListWidget(true);
		}
	}
		break;
	case EMouseCursorNeededWidgetType::InviteFriendWidget:
	{
		if (_Visible)
		{
			// Menu를 이미 보고 있는 중이라면 따로 처리 x (Menu 창에 해당 중복된 기능이 이미 있음)
			if (m_MenuWidget->GetVisibility() == ESlateVisibility::Visible) break;
			m_MainHUDWidget->ToggleFriendListWidget(true); // Menu를 보고 있지 않은 중이라면, 비로소 FriendList 활성화
		}
		else m_MainHUDWidget->ToggleFriendListWidget(false);
		
		break;
	}
	case EMouseCursorNeededWidgetType::GameOverWidget:
	{
		if (_Visible) m_MainHUDWidget->GetGameOverWidget()->ActivateGameOverSequence(); 
		else		  m_MainHUDWidget->GetGameOverWidget()->SetVisibility(ESlateVisibility::Collapsed);		  
		
		break;
	}
	case EMouseCursorNeededWidgetType::SkinWidget:
	{
		m_SkinWidget->SetVisibility(_Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		break;
	}
	case EMouseCursorNeededWidgetType::End:
	{
		UC_Util::Print("[AC_UIManager::ToggleMouseInputNeededWidgetVisibility] : Invalid Widget type received", FColor::Red, 10.f);
		return;
	}
	}

	for (const TPair<EMouseCursorNeededWidgetType, UUserWidget*>& WidgetVisibility : m_MouseCursorNeededWidgets)
	{
		// 마우스 관련 Widget 중 하나라도 켜져있는 상태인 경우, 마우스 커서 보이기 처리
		if (WidgetVisibility.Value->GetVisibility() == ESlateVisibility::Visible)
		{
			FInputModeGameAndUI InputMode{};
			//InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
			InputMode.SetWidgetToFocus(nullptr); // 이거 문제가 될수 있나 싶다

			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
			
			return;
		}
	}
	
	// 모두 꺼져있는 상태인 경우, Mouse Cursor 비활성화, 게임 전용 입력 모드로 복귀
	FInputModeGameOnly InputMode{};
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(false);
}

void AC_UIManager::ToggleMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType _WidgetToToggle)
{
	if (!m_MouseCursorNeededWidgets.Contains(_WidgetToToggle))
	{
		UC_Util::Print("[AC_UIManager::ToggleMouseInputNeededWidgetVisibility] : Invalid Widget type received", FColor::Red, 10.f);
		return;
	}
	
	ToggleMouseInputNeededWidgetVisibility
	(
		_WidgetToToggle,
		!GetMouseInputNeededWidgetVisibility(_WidgetToToggle)
	);
}
