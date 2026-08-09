// Fill out your copyright notice in the Description page of Project Settings.


#include "C_UIManager.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/MenuUI/C_MenuWidget.h"
#include "Utility/C_Util.h"

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
