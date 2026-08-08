
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_BasicPlayerInputComponent.generated.h"

class AC_BasicPlayer;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EPlayerIMCMode : uint8
{
	DefaultMapping,
	OnlyMovementMapping // 거점 활성화하는 인원의 경우, 해당 Mapping 모드 사용할 것
};

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
	
protected:
	
	// Player MovementOnly InputMappingContext
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Input")
	UInputMappingContext* OnlyMovementMappingContext{};
	
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
	
	const UInputAction* FindIAByName(const FString& _Name);

public:
	
	/// <summary>
	/// IMC 모드 변경 
	/// </summary>
	void SetPlayerIMCMode(EPlayerIMCMode _IMCMode);
	
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
	void SwitchFireModeAction();
	
	void KeepAimActionStart();
	void KeepAimActionEnd();
	
	// 인벤토리를 여닫는 함수
	void ToggleInventoryWidget();

private: // Equip Weapon input 관련

	void EquipMainWeapon();
	void EquipMeleeWeapon();
	void EquipThrowable(); // TODO : Throwable Equip은 따로 처리를 안할수도
	void EquipPotion(); // TODO : Throwable Equip은 따로 처리를 안할수도
	
	void ToggleArmed();

private: // FreeLook 관련
	
	void FreeLookHoldStart();
	void FreeLookHoldEnd();
	
private: // Ping system 관련
	
	void MarkPing();
	
};
