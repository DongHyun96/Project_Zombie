// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_CrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 우클릭 시 호출 
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void ZoomIn();

	// 우클릭 해제 시 호출
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void ZoomOut();

protected:

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_AimZoom;

};
