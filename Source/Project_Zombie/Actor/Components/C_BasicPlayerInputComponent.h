
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

	// Default mapping context와 연결된 InputAction들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	TMap<FString, const class UInputAction*> m_mapIA{};
	
private:
	// 조종할 대상 캐릭터와 무브먼트 컴포넌트 주소 저장용
	UPROPERTY()
	AC_BasicPlayer* Player;

	UPROPERTY()
	class UCharacterMovementComponent* PlayerMovement;

protected:
	float	AimPressStartTime = 0.f;
	float	HoldThreshold = 0.1f;       // 꾹 누르기 판정 기준 시간 (0.1초)
	bool	bIsShoulderToggled = false; // 견착 토글 상태 추적
	bool	bIsHoldFired = false;       // 꾹 누르기가 이미 발동했는지 여부

	FTimerHandle AimHoldTimerHandle;

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
	
	const UInputAction* FindIAByName(const FString& _Name);
	
private:
	
	void MoveAction(const FInputActionValue& Value);
	
	void SprintStart();
	void SprintEnd();

	void LookAction(const FInputActionValue& Value);
	void JumpAction();
	void CrouchAction();

	void InteractionAction();
	
	
	void FireStarted();
	void FireOnGoing();
	void FireEnd();
	void ReloadAction();
	
	void KeepAimActionStart();
	void KeepAimActionOngoing();
	void KeepAimActionEnd();
	
	// 인벤토리를 여닫는 함수
	void ToggleInventoryWidget();

private: // Equip Weapon input 관련

	void EquipMainWeapon();
	void EquipMeleeWeapon();
	void EquipThrowable(); // TODO : Throwable Equip은 따로 처리를 안할수도
	void ToggleArmed();

private: // FreeLook 관련
	
	void FreeLookHolStart();
	void FreeLookHoldEnd();
	
private: // Ping system 관련
	
	void MarkPing();
	
};
