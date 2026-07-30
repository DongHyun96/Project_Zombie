// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/ItemDetails/C_MattersWidget.h"

#include "GameModeAndManager/C_ItemManager.h"
#include "C_MatterRowWidget.h"
#include "Components/ScrollBox.h"
#include "Utility/C_Util.h"


void UC_MattersWidget::UpdateWidget(TArray<const FName> InItemRowNames)
{
	MattersScrollBox->ClearChildren();
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	for (const auto& it : InItemRowNames)
	{
		const FItemData* Data = ItemManager->GetItemData<FItemData>(EItemTableType::General, it);
		
		if (!Data)
		{
			UC_Util::Print("FItemData Is Nullptr!");
			continue;
		}
		
		UC_MatterRowWidget* MatterRow = CreateWidget<UC_MatterRowWidget>(this, MatterRowWidgetClass);
		
		if (!MatterRow)
		{
			UC_Util::Print("MatterRowWidget Is Nullptr!");
			continue;
		}
		
		// TODO : 필요한 Matter의 Count를 어디에선가 찾아 와서 넣어 주어야 함.(강화 테이블? 같이 강화당 필요한 재화량을 기록한 무언가 필요)
		MatterRow->UpdateWidget(Data->IconTexture.Get(), Data->ItemName, -1);// TODO : Count는 우선 -1로 초기화.
		
		MattersScrollBox->AddChild(MatterRow);
	}	
	
}
