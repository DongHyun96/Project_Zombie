// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/PlayerStat/C_PlayerUpMattersWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/ScrollBox.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"
#include "UI/InvenUI/Upgrade/ItemDetails/C_MatterRowWidget.h"
#include "Utility/C_Util.h"

void UC_PlayerUpMattersWidget::UpdateWidget(AC_BasicPlayer* InUsePlayer)
{
	PlayerUpMattersScrollBox->ClearChildren();
	m_MatterRows.Empty();
	
	if (!InUsePlayer) return;
	
	// TODO : Player의 스탯컴포넌트를 가져와서 이름과 현재 Grade를 활용해서 FPlayerStatUpgradeData에서 요구되는 재료들을 가져와서 MatterRow에 UpateWidget의
	// 매개 변수로 넣어주어야 함.
	
	// PlayerStatUpgradeWidget의 SelectedStatName의 Grade와 강화를 위한 재료만 표시하면 되지.
	UC_StatComponentBase* StatComp = InUsePlayer->GetStatComponent();
	
	if (!StatComp) return;
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	FName TargetStatName = PlayerStatUpgradeWidget->GetSelectedStatName();
	
	if (!TargetStatName.IsValid()) return;
	
	const uint8 StatGrade = StatComp->GetStatGrade(TargetStatName);
	
	const FPlayerStatUpgradeData* PSUpData = ItemManager->GetPlayerStatUpgradeData(TargetStatName);
	
	if (!PSUpData)
	{
		UC_Util::Print("FPlayerStatUpgradeData Is Nullptr~!");
		if (PlayerStatUpgradeWidget)
			PlayerStatUpgradeWidget->SetHasRequiredItems(false);
		
		PlayerStatUpgradeWidget->UpdateButton(false);
		
		return;
	}
	
	const TArray<FGradeCostInfo>& CostInfoArr = PSUpData->GradeCost; 
	
	if (StatGrade >= CostInfoArr.Num())
	{
		PlayerStatUpgradeWidget->UpdateButton(false);
		return; // TODO : 버튼 닫아 주어야 함.
	}
	
	// StatGrade의 시작은 0, CostInfoArr의 TargetGrade는 1부터 시작이라 다음 강화 단계를 불러옴.
	const FGradeCostInfo& CostInfo = CostInfoArr[StatGrade];
	
	if (StatGrade >= CostInfo.TargetGrade)
	{
		PlayerStatUpgradeWidget->UpdateButton(false);
		return; // TODO : 버튼 닫아 주어야 함.
	}
	const TArray<FUpgradeMaterialInfo>& RequiredMaterials =  CostInfo.RequiredMaterials;
	
	UC_InvenComponent* InvenComp = InUsePlayer->GetInvenComponent();
	
	bool HasRequiredItems = true;
	
	for (const auto& MaterialInfo : RequiredMaterials)
	{
		if (MaterialInfo.MatterItemID.IsNone() || MaterialInfo.RequiredCount <= 0) continue;
		
		const FName& MaterialName = MaterialInfo.MatterItemID;
		
		const int32& count = MaterialInfo.RequiredCount;
		
		const FItemData* ItemData = ItemManager->GetItemData<FItemData>(EItemTableType::General, MaterialName);
		
		if (!ItemData) continue;
		
		UC_Util::Print(MaterialName.ToString());
		
		// 플레이어가 가방에 갖고 있는 해당 재료 수량 계산
		int32 HasCount = InvenComp ? InvenComp->GetTotalItemCount(MaterialName) : 0;
		
		UC_MatterRowWidget* MatterRow = CreateWidget<UC_MatterRowWidget>(this, MatterRowWidgetClass);
		if (MatterRow)
		{
			if (HasCount < count && PlayerStatUpgradeWidget)
			{
				HasRequiredItems = false;
			}
			
			UTexture2D* LoadedTexture = ItemData->IconTexture.Get();
			if (!LoadedTexture && !ItemData->IconTexture.IsNull())
			{
				LoadedTexture = ItemData->IconTexture.LoadSynchronous(); // 동기 로드
			}
			
			MatterRow->UpdateWidget(
				LoadedTexture,
				ItemData->ItemName,
				HasCount,
				count
			);
			PlayerUpMattersScrollBox->AddChild(MatterRow);
			m_MatterRows.Add(MatterRow);
		}
		else
		{
			HasRequiredItems = false;
		}
	}
		
	if (PlayerStatUpgradeWidget)
	{
		PlayerStatUpgradeWidget->SetHasRequiredItems(HasRequiredItems);
		PlayerStatUpgradeWidget->UpdateButton(true);
	}
}
