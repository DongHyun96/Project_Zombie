// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "C_UIManager.generated.h"

/// <summary>
/// Debug 메시지 용
/// </summary>
struct FLocalDebugMessage
{
	FString Text{};
	FColor Color{};
	float ExpireTime{}; // 화면에서 사라질 시간
};

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_UIManager : public AHUD
{
	GENERATED_BODY()

public:
	
	static AC_UIManager* Get(const UWorld* _World);
	
	static class UC_GameMainHUD* GetMainHUD(const UWorld* _World);
	
public:
	
	virtual void BeginPlay() override;

	virtual void DrawHUD() override;
	
public:

	// UC_GameMainHUD* GetMainHUDWidget() const { return m_MainHUDWidget; }
	
	class UC_InventoryWidget* GetInventoryWidget() const { return m_InventoryWidget; }
	
	class UC_MenuWidget* GetMenuWidget() const { return m_MenuWidget; };

public:
	
	void PrintLocalDebugMessage(const FString& _Message, const FColor& _Color, float _Duration);
	
protected:

	// HUD로 사용할 최상위 UUserWidget 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> m_MainHUDClass{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UC_InventoryWidget> m_InventoryWidgetClass{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UC_MenuWidget> m_MenuWidgetClass{};
protected:

	// 생성된 MainHUD Widget 객체
	UPROPERTY(BlueprintReadOnly)
	UC_GameMainHUD* m_MainHUDWidget{};

	// 생성된 Inventory Widget 객체
	UPROPERTY(BlueprintReadOnly)
	UC_InventoryWidget* m_InventoryWidget{};

	// 생성된 Inventory Widget 객체
	UPROPERTY(BlueprintReadOnly)
	UC_MenuWidget* m_MenuWidget{};
	
	
	
private:
	
	TArray<FLocalDebugMessage> m_DebugMessages{};
};

#define UI_MANAGER(_World) AC_UIManager::Get(_World)
#define MAIN_HUD(_World) AC_UIManager::GetMainHUD(_World)

/// <summary>
/// 리슨 서버 환경에서의 프린트 출력  
/// </summary>
#if !UE_BUILD_SHIPPING
#define PRINT_LOCAL(_World, _Msg, _Color, _Duration) \
do { \
if (AC_UIManager* UIMgr = UI_MANAGER(_World)) \
{ \
UIMgr->PrintLocalDebugMessage(_Msg, _Color, _Duration); \
} \
} while (0)
#else
#define PRINT_LOCAL(_World, _Msg, _Color, _Duration) do {} while (0)
#endif

