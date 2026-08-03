// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PlayerStatUpgradeWidget.generated.h"

class AC_BasicPlayer;
class UC_PlayerUpMattersWidget;
class UC_PlayerStatsWidget;
class UImage;
class UTextBlock;
class UC_ItemStatsWidget;
class UC_MattersWidget;
class UButton;
class UC_SelectedStatWidget;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerStatUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()
		
public:
	void SetSelectedStatName(const FText& InSelectedStatName);
	
	void SetUsePlayer(AC_BasicPlayer* InUsePlayer) { m_UsePlayer = InUsePlayer; }
	
	void InitWidget();
protected:
	virtual void NativeOnInitialized() override;
	
	
protected:
	// 현재 강화하고자 하는 아이템의 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PlayerName = nullptr;	
	
	// 강화창 닫기 버튼.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* ExitButton = nullptr;
	
	// 현재 강화하고자 하는 아이템의 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* PlayerIcon = nullptr;
	
	// 강화 가능한 스탯들, 여기서 스탯 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_PlayerStatsWidget* PlayerStatsWidget = nullptr;
	
	// 강화에 필요한 능력치.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_PlayerUpMattersWidget* PlayerUpMattersWidget = nullptr;
	
	// 업그레이드 실행 버튼, 서버에 요청하는 버튼.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* UpgradeBtn = nullptr;
	
	// 선택한 Stat의 현재와 다음 Stat을 보여줌.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_SelectedStatWidget* SelectedStatRow = nullptr;
	
protected:
	// 선택된 스탯의 이름.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SelectedStatName{};
	
	// 로컬에서 현재 업그레이드 중인지 판단하는 변수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUpgrading = false;
	
	// 강화에 필요한 아이템들을 가지고 있는가?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasRequiredItems = false;
	
	UPROPERTY()
	TObjectPtr<AC_BasicPlayer> m_UsePlayer = nullptr;
};
