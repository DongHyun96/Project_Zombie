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

UENUM(BlueprintType)
enum class EMouseCursorNeededWidgetType : uint8
{
	InvenWidget,
	MenuWidget,
	InviteFriendWidget,
	GameOverWidget,
	SkinWidget,
	End UMETA(Hidden)	
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
	
	class UC_MenuWidget* GetMenuWidget() const { return m_MenuWidget; }

	class UC_SkinWidget* GetSkinWidget() const { return m_SkinWidget; }

public:
	
	void PrintLocalDebugMessage(const FString& _Message, const FColor& _Color, float _Duration);

public:

	bool GetMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType _WidgetType) const;

	/// <summary>
	/// <para> 마우스 Input이 필요한 Widget 종류에 대한 Visibility Toggle 통합 </para>
	/// <para> 통합한 이유 : InputMode 꼬임 방지 </para>
	/// </summary>
	/// <param name="_WidgetToToggle"> : Toggle시킬 Widget 종류 </param>
	/// <param name="_Visible"> : true일 경우 보이게끔 처리 (반대는 안 보이게끔) </param>
	void ToggleMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType _WidgetToToggle, bool _Visible);

	/// <summary>
	/// <para> 마우스 Input이 필요한 Widget 종류에 대한 Visibility Toggle 통합 </para>
	/// <para> 현재 Visible 상태에서 반대로 Toggle 처리 시킨다 </para>
	/// <para> 통합한 이유 : InputMode 꼬임 방지 </para>
	/// </summary>
	/// <param name="_WidgetToToggle"> : Visible 반대로 Toggle시킬 Widget 종류 </param>
	void ToggleMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType _WidgetToToggle);
	
	void SetEnteredStorage(class AC_Storage* _Storage) { m_EnteredStorage = _Storage; }
	void SetEnteredInviteInteractable(class AC_InviteInteractable* _InviteInteractable) { m_EnteredInviteInteractable = _InviteInteractable; } 
	
protected:

	// HUD로 사용할 최상위 UUserWidget 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> m_MainHUDClass{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UC_InventoryWidget> m_InventoryWidgetClass{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UC_MenuWidget> m_MenuWidgetClass{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UC_SkinWidget> m_SkinWidgetClass{};

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
	
	UPROPERTY(BlueprintReadOnly)
	UC_SkinWidget* m_SkinWidget{};

private:
	
	/* 현재 마우스 커서 Input이 필요한 Widget이 켜져 있는지 체크용 */
	UPROPERTY()
	TMap<EMouseCursorNeededWidgetType, UUserWidget*> m_MouseCursorNeededWidgets{};
	
private: /* Interact 가능한(Collider에 접근한) Storage(Weapon Cabinet) 관련 */
	
	UPROPERTY()
	AC_Storage* m_EnteredStorage{};
	
	UPROPERTY()
	AC_InviteInteractable* m_EnteredInviteInteractable{};
	
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

