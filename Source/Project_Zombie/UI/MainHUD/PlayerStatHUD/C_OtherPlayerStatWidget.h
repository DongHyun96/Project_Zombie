// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_OtherPlayerStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_OtherPlayerStatWidget : public UUserWidget
{
	GENERATED_BODY()
	
	friend class UC_PlayerStatComponent;
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	

public:
	
	/// <summary>
	/// Other 플레이어 등록 -> 주의 : 해당 플레이어의 정보가 완전히 초기화된 상태에서 호출하여 등록 처리할 것
	/// 이미 등록 처리된 Player의 경우, 정보만 다시 업데이트
	/// </summary>
	/// <param name="_Player"></param>
	void RegisterOtherPlayer(class AC_BasicPlayer* _Player);

	void UpdateHPBar(AC_BasicPlayer* _TargetPlayer, float _HPRatio);

protected:
	
	UPROPERTY(meta = (BindWidget))
	class UC_MiniHPBarWidget* MiniHP_0{};
	
	UPROPERTY(meta = (BindWidget))
	UC_MiniHPBarWidget* MiniHP_1{};
	
	UPROPERTY(meta = (BindWidget))
	UC_MiniHPBarWidget* MiniHP_2{};
	
	UPROPERTY(meta = (BindWidget))
	UC_MiniHPBarWidget* MiniHP_3{};

private:

	UPROPERTY()
	TArray<UC_MiniHPBarWidget*> m_MiniHPs{};
	
private:

	// 등록 처리된 Player들의 Widget
	UPROPERTY()
	TMap<AC_BasicPlayer*, UC_MiniHPBarWidget*> m_RegisteredWidget{};


};
