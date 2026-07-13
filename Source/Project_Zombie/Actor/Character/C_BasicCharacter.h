// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C_BasicCharacter.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AC_BasicCharacter();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:

	// Player 및 Enemy 생성자에서 자기자신에게 맞는 StatComponent 생성 처리 중
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "StatComponent"))
	class UC_StatComponentBase* m_StatComponent{};
};
