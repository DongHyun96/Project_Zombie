// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "C_MontagePrioritySettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(MONTAGE_PRIORITY, Log, All);

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, Meta=(DisplayName="AnimMontage priority settings"))
class PROJECT_ZOMBIE_API UC_MontagePrioritySettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UC_MontagePrioritySettings();

	virtual FText GetSectionDescription() const override;
	
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

private:
	
	void OnEngineInitComplete();
	void SyncMontagePriorityTags();
	
#endif

public:
	
	/// <summary>
	/// 입력으로 들어온 PriorityTag의 PriorityValue값 구하기
	/// </summary>
	/// <param name="_MontagePriorityTag"> : PriorityValue를 구할 MontagePriorityGameTag값 </param>
	/// <param name="_OutPriorityValue"> : 구한 PriorityValue 레퍼런스 return </param>
	/// <returns> : PriorityMap에 있지 않은 GameplayTag의 경우 return false(Invalid GameplayTag received) </returns>
	bool GetPriority(const FGameplayTag& _MontagePriorityTag, OUT uint8& _OutPriorityValue) const;
    
protected:
	
	UPROPERTY(Config, EditAnywhere, Category = "AnimMontage Priority List", meta = (ReadOnlyKeys, EditFixedSize))
	TMap<FGameplayTag, uint8> PriorityMap{};
};
