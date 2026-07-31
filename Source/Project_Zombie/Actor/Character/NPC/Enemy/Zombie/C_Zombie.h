// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../C_BasicEnemy.h"
#include "C_Zombie.generated.h"

UENUM(BlueprintType)
enum class EZombieType : uint8
{
	NormalZombie,
	ToxicZombie,
	NurseZombie,
	CopZombie,
	TankZombie
};

UCLASS()
class PROJECT_ZOMBIE_API AC_Zombie : public AC_BasicEnemy
{
	GENERATED_BODY()

protected:
	
	const EZombieType m_ZombieType{};

public:
	/// <summary>
	/// 현재 Zombie 종류 반환
	/// </summary>
	EZombieType GetZombieType() const { return m_ZombieType; }
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// 기본 CDO 생성 처리를 위해 기본생성자는 항상 있어야 함
	/// </summary>
	AC_Zombie();

	/// <summary>
	/// 자식 Zombie 클래스에서 자신의 ZombieType을 초기화하기 위한 생성자
	/// </summary>
	AC_Zombie(EZombieType _ZombieType);
};
