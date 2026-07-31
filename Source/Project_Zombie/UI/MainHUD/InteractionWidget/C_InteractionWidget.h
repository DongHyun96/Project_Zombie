// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_InteractionWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UImage;
class UMaterialInstanceDynamic;

/**
상호작용 Widget (F 키)
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_InteractionWidget : public UUserWidget
{
	GENERATED_BODY()


protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	/// <summary>
	/// 상호작용 UI 활성화 
	/// </summary>
	/// <param name="_InteractionText">상호작용 설명 Text</param>
	void ActivateInteraction(const FText& _InteractionText);

	/// <summary>
	/// 상호작용 UI 비활성화
	/// </summary>
	void DeactivateInteraction();

public:
	/// <summary>
	/// 상호작용 시 Timer 활성화
	/// </summary>
	/// <param name="_Duration">상호작용 Time</param>
	void ActivateInteractionTimer(float _Duration);

	/// <summary>
	///	상호작용 종료 시 Timer 비활성화
	/// </summary>
	void DeactivateInteractionTimer();



protected:

	// Interaction UI Panel
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InteractionPanel{};
	
	// 상호작용 설명 Text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Interaction{};

	// Interaction Timer Panel
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* TimerPanel{};

	UPROPERTY(meta = (BindWidget))
	UImage* Image_InteractionTimer{};

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* m_TimerMaterialInstance{};

private:

	float m_TimerDuration = 0.0f;
	float m_TimerElapsedTime = 0.0f;
};
