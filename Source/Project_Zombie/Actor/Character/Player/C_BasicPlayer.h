// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "C_BasicPlayer.generated.h"

// 캐릭터 상태
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Idle,
	Dead,
};

// 손 상태
UENUM(BlueprintType)
enum class EHandState : uint8
{
	UnArmed,
	WeaponGun,
	WeaponMelee,
	WeaponThrowable,
};

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicPlayer : public AC_BasicCharacter
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "SpringArm"))
	class USpringArmComponent* m_SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "MainCamera"))
	class UCameraComponent* m_Camera;

// 캐릭터 상태 // 이건 나중에 BasicCharacter 로 옮겨도?
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerState		m_PlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float				m_BaseMaxSpeed;

// InputContainer 를 통해서 입력을 받을지...
// Input
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// 이동			// WASD
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Move;

	// 시점 회전		// Mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Look;

	// 점프			// SpaceBar
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Jump;

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

protected:
	void MoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);
	void JumpAction();
	void FireAction();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
