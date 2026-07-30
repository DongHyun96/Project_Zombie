// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_ItemUpgradeWidget.generated.h"

class AC_BasicPlayer;
class UC_SelectedStatWidget;
class UC_MattersWidget;
class UC_ItemStatsWidget;
class UImage;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ItemUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
public:
	// 현재 아이템에 따라 위젯 업데이트
	void UpdateWidget();
	
	// 아이템 강화 요청
	void RequestItemUpgrade();
	
	
public:
	void SetUsePlayer(AC_BasicPlayer* InUsePlayer) { m_UsePlayer = InUsePlayer; }
	
protected:
	// 현재 강화하고자 하는 아이템의 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemName = nullptr;	
	
	// 강화창 닫기 버튼.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* ExitButton = nullptr;
	
	// 현재 강화하고자 하는 아이템의 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemIcon = nullptr;
	
	// TODO : 이 부분은 최종 능력치를 보여주는 부분으로 바꾸는게 나을 듯. ScrollBox로 능력치 보여주면 될 듯?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemDesc = nullptr;
	
	// 강화 가능한 스탯들, 여기서 스탯 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_ItemStatsWidget* ItemStats = nullptr;
	
	// 강화에 필요한 능력치.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_MattersWidget* Matters = nullptr;
	
	// 업그레이드 실행 버튼, 서버에 요청하는 버튼.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* UpgradeBtn = nullptr;
	
	// 선택한 Stat의 현재와 다음 Stat을 보여줌.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_SelectedStatWidget* SelectedStatRow = nullptr;
	
protected:
	// 현재 강화 하고자 하는 아이템.
	// 이걸 Entry째로 들고 있을 지 아니면 그냥 슬롯 Idx만 들고 있게해서 Player에게 조회하게 할 지 고민.
	UPROPERTY()
	int32 DroppedItemSlotIdx{};
	
	UPROPERTY()
	TObjectPtr<AC_BasicPlayer> m_UsePlayer = nullptr;
	
	// 이건 UsePlayer의 InteractionComponent를 통해 대체 할 수 있다면 사용하지 않을 수 있음.
	AActor* InteractingActor = nullptr;
};
