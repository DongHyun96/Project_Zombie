// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatComponent/C_StatComponentBase.h"
#include "C_PlayerStatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PlayerStatComponent : public UC_StatComponentBase
{
	GENERATED_BODY()

public:
	
	UC_PlayerStatComponent();

public:
	
	virtual void BeginPlay() override;
	
private:
	
	virtual UScriptStruct* GetStatDataStruct() const override;
	
private:
	
	/// <summary>
	/// HP 업데이트 시, 호출될 함수 (UI 업데이트 처리)
	/// </summary>
	void OnHPUpdate(float _CurHPRatio);

private:
	
	UPROPERTY()
	class UC_GameMainHUD* m_MainHUD{};
	
};


/*// 내 Player일 때에 나의 HUD를 업데이트 시켜주어야 함 -> 주의 : IsLocallyControlled() 여기서 사용 못할수도? TakeDamage 자체가 서버 쪽에서 실행되는 함수라고 함 (AI 피셜이라 확인해봐야 함)
if (IsLocallyControlled())
{
	AC_UIManager* UIManager = Cast<AC_UIManager>(GetController<APlayerController>()->GetHUD());
	UIManager->GetMainHUDWidget()->UpdateHPBarRatio(m_StatComponent->GetCurHPRatio());
}*/