// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_InteractionWidget.generated.h"

class UCanvasPanel;
class UTextBlock;

/**
상호작용 Widget (F 키)
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_InteractionWidget : public UUserWidget
{
	GENERATED_BODY()

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


protected:

	// Interaction UI 전체 Panel
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InteractionPanel{};
	
	// 상호작용 설명 Text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Interaction{};
};
