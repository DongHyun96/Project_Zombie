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

/* 
 * Tab키 전용 Interaction instruction panel 활성화/비활성화
 * 위의 메서드들과 별개로 따로 뺀 이유는 Panel 자체를 새로 생성했기 때문 (비율 등의 문제로 Text 내용만 바꿔칠 수 없었음) 
 */
	
public:
	
	void ToggleTabInteractionInstruction(bool _Visible);	
	
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
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* TabInteractPanel{};

private:

	float m_TimerDuration = 0.0f;
	float m_TimerElapsedTime = 0.0f;
};
