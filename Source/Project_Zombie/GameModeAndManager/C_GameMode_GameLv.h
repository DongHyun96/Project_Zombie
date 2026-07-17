// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "C_GameMode_GameLv.generated.h"

/**
 * GameLevel에서 사용될 GameMode Base class
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_GameMode_GameLv : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	/*
	 *TODO : 로그아웃시 잠금된 아이템 슬롯을 해제해주는 기능만 구현되어 있음. 
	 * 근데 컨트롤러쪽에서 따로 구현해두었는데 나중에 실제 Release버전에서 다시 한번 조정해야 할 수 있음.
	 */
	virtual void Logout(AController* Exiting) override;
};
