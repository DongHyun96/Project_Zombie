
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_BasicPlayerInputComponent.generated.h"

// 이전 프로젝트에서 있길래 일단 가져왔는데 어디 쓰던건지 까먹음, TODO : 알아보고 필요하면쓰고 아니면 지우기
//UENUM(BlueprintType)
//enum class EInputAction : uint8
//{
//	MOVE,
//	JUMP,
//	LOOK,
//	WALK,
//	SPRINT,
//	CRAWL, 
//	CROUCH,
//	HOLD_DIRECTION,
//	DRAW_MAIN_WEAPON,
//	DRAW_SUB_WEAPON,
//	DRAW_MELEE_WEAPON,
//	DRAW_THROWABLE_WEAPON,
//	TOGGLE_ARMED,
//	CHANGE_SHOOTING_MODE,
//	RELOAD,
//	SHOT,
//	TOGGLE_AIMING,
//	PING,
//	INTERACTION,
//	MINI_MAP,
//	INVEN_UI,
//	MAIN_MENU,
//	THROWABLE_WHEEL,
//	CONSUMABLE_WHEEL
//};

class AC_BasicPlayer;
struct FInputActionValue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_BasicPlayerInputComponent : public UActorComponent
{
	GENERATED_BODY()

	// 이하 멤버 변수
protected: 
	// Player Main InputMappingContext
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	class UInputMappingContext* DefaultMappingContext{};

	// 이동			// WASD
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Move;

	// 시점 회전		// Mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Look;

	// 점프			// SpaceBar
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Jump;

	// 웅크리기			// Ctrl
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Crouch;

	// 공격			// LMB
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Fire;

	// 시점 전환		// RMB
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Aim;

	// 재장전		// R
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Reload;

	// 주무기 1 장착	// 1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_EquipPrimary1;

	// 주무기 2 장착	// 2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_EquipPrimary2;

	// 보조무기 장착	// 3
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_EquipMelee;

	// 투척류 무기 장착 // 4
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_EquipThrowable;

	// 인벤토리 열기	// Tab
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ToggleInventory;

	// 상호작용		// E
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Interact;

	// 인칭 전환
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	//UInputAction* IA_ToggleMode;

private:
	// 조종할 대상 캐릭터와 무브먼트 컴포넌트 주소 저장용
	UPROPERTY()
	AC_BasicPlayer* Player;

	UPROPERTY()
	class UCharacterMovementComponent* PlayerMovement;

protected:
	//TODO : 알아보고 필요하면쓰고 아니면 지우기
	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//TMap<EInputAction, UInputAction*> InputActionMap{};

	// 이하 멤버 함수
public:	
	UC_BasicPlayerInputComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 플레이어의 SetupPlayerInputComponent에서 호출해 줄 초기화 함수
	void InitializePlayerInput(UInputComponent* PlayerInputComponent, AC_BasicPlayer* InPlayer);

private:
	void MoveStart(const FInputActionValue& Value);
	void MoveAction(const FInputActionValue& Value);
	void MoveEnd(const FInputActionValue& Value);
	
	void LookAction(const FInputActionValue& Value);
	void JumpAction();
	void CrouchAction();
	void FireAction();
};
