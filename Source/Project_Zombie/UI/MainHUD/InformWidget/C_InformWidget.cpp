// Fill out your copyright notice in the Description page of Project Settings.


#include "C_InformWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Utility/C_Util.h"

void UC_InformWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Init PlayerWarningLog 관련
	for (int i = 0; i < 4; ++i)
	{
		FString PlayerWarningWidgetName = FString::Printf(TEXT("PlayerWarningLogText%d"), i);
		FName WidgetName(*PlayerWarningWidgetName);
		UTextBlock* TextBlock = Cast<UTextBlock>(GetWidgetFromName(WidgetName));
		if (!TextBlock)
		{
			UC_Util::Print("From Instruction Widget NativeConstruct : TextBlock casting failed!", FColor::Red, 10.f);
			continue;
		}
 
		UCanvasPanelSlot* PlayerWarningLogTextPanel = Cast<UCanvasPanelSlot>(TextBlock->Slot);
        
		PlayerWarningLogTexts.Add(TextBlock);
		PlayerWarningLogTextPanels.Add(PlayerWarningLogTextPanel);
		PlayerWarningLogEachPositions.Add(PlayerWarningLogTextPanel->GetPosition());

		PlayerWarningLogSequence.Add(i);
	}

	/*FTimerDelegate TempDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (!LEVEL_MANAGER) return;
		AC_BasicPlayer* LocalPlayer = LEVEL_MANAGER->GetLocalPlayer();
		
		if (!LocalPlayer) return;
		LocalPlayer->GetInvenComponent()->OnInventorySlotChanged.AddDynamic(this, &UC_InformWidget::OnInvenSlotChanged);
		
		GetWorld()->GetTimerManager().ClearTimer(m_TimerInvenGameLogRegister);
	});
	
	GetWorld()->GetTimerManager().SetTimer(m_TimerInvenGameLogRegister, TempDelegate, 0.1f, true);*/
}

void UC_InformWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_InformWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	HandleLogFadeOut(InDeltaTime);
	HandleLogQueuePositionsAndDefaultAlpha(InDeltaTime);
}

bool UC_InformWidget::AddPlayerWarningLog(const FString& WarningLog, const FColor& _LogColor)
{
	// 새로운 첫 번째 로그로 맨 뒤 로그 이동시키기
	int TargetIndex = PlayerWarningLogSequence.Last();
	PlayerWarningLogSequence.RemoveAt(PlayerWarningLogSequence.Num() - 1);
	PlayerWarningLogSequence.Insert(TargetIndex, 0);
	
	/////////////////////////////////////////////////////////////////////
    
	UTextBlock* TargetTextBlock = PlayerWarningLogTexts[TargetIndex]; 

	// 내용 setting 하기
	TargetTextBlock->SetText(FText::FromString(WarningLog));

	TargetTextBlock->SetColorAndOpacity(_LogColor);
	
	// Render Alpha값 기본 값으로 지정
	TargetTextBlock->SetRenderOpacity(1.f);

	// 초기 위치 지정
	PlayerWarningLogTextPanels[TargetIndex]->SetPosition(PlayerWarningLogEachPositions[0]);
    
	// Maximum 3초의 Log LifeTime 처리
	ApplyNewLifeTimerToLog(TargetTextBlock, 3.f);
    
	return true;
}

bool UC_InformWidget::AddPlayerWarningLog(const FName& _ItemRowName, int32 _ItemPickUpCount)
{
	const FString* TargetItemName = m_ItemNameMap.Find(_ItemRowName);
	if (!TargetItemName) return false;

	
	AddPlayerWarningLog("ADDED TO INVENTORY : " + *TargetItemName + " x " + FString::FromInt(_ItemPickUpCount), FColor::White);
	
	return true;
}

bool UC_InformWidget::AddEquippedWeaponLog(const FName& _WeaponItemRowName)
{
	const FString* pWeaponItemName = m_ItemNameMap.Find(_WeaponItemRowName);
	if (!pWeaponItemName) return false;

	AddPlayerWarningLog("EQUIPPED : " + *pWeaponItemName, FColor::White);
	return true;
}

void UC_InformWidget::ToggleGameStartPanel(bool _Visible)
{
	GameStartsTimerPanel->SetVisibility(_Visible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UC_InformWidget::UpdateGameStartLeftTime(int32 _Time)
{
	GameStartTimerText->SetText(FText::AsNumber(_Time));
}

void UC_InformWidget::ShowMainInstruction(const FString& _Construction)
{
	MainInstructionText->SetText(FText::FromString(_Construction));
	PlayAnimation(ShowMainInstructionAnim);
}

void UC_InformWidget::HandleLogFadeOut(const float& DeltaTime)
{
	for (TSet<UWidget*>::TIterator It(FadeOutLogs); It; ++It)
	{
		UWidget* Log = *It;
		const float CurrentAlpha = Log->GetRenderOpacity();

		if (CurrentAlpha < 0.05f) // Alpha가 충분히 작아서 Destination에 도달했다고 판단
		{
			Log->SetRenderOpacity(0.f);
			It.RemoveCurrent();
			continue;
		}

		// Lerp Alpha color to 0.f
		const float DestAlpha = FMath::Lerp(CurrentAlpha, 0.f, DeltaTime * 10.f);
		Log->SetRenderOpacity(DestAlpha);
	}
}

void UC_InformWidget::HandleLogQueuePositionsAndDefaultAlpha(const float& DeltaTime)
{
	for (int i = 1; i < PlayerWarningLogSequence.Num(); ++i)
	{
		const int Index = PlayerWarningLogSequence[i];
		UCanvasPanelSlot* CanvasPanelSlot = PlayerWarningLogTextPanels[Index];

		const FVector2D CurrentPosition = CanvasPanelSlot->GetPosition();
		const FVector2D DestPosition = FMath::Lerp(CurrentPosition, PlayerWarningLogEachPositions[i], 20.f * DeltaTime);
		CanvasPanelSlot->SetPosition(DestPosition);
	}
	
	// 마지막에서 두 번째 -> 로그는 보이는데, FadeOut중이 아니라면 FadeOut 처리
	const int LastPrevIndex    = PlayerWarningLogSequence[PlayerWarningLogSequence.Num() - 2];
	UWidget* LastPrevLogWidget = PlayerWarningLogTexts[LastPrevIndex];

	if (LastPrevLogWidget->GetRenderOpacity() > 0.f) // 아직 보이는 중이고
	{
		if (!FadeOutLogs.Contains(LastPrevLogWidget)) // Start Fade 처리가 안된 상황
		{
			if (FTimerHandle* ExistingTimer = LogLifeTimers.Find(LastPrevLogWidget))
				GetWorld()->GetTimerManager().ClearTimer(*ExistingTimer);
			LogLifeTimers.Remove(LastPrevLogWidget);
			StartFadeOut(LastPrevLogWidget);
		}
	}
	
	/*for (int i = 1; i < PlayerWarningLogSequence.Num(); ++i)
	{
		const int Index = PlayerWarningLogSequence[i];
		UCanvasPanelSlot* CanvasPanelSlot = PlayerWarningLogTextPanels[Index];

		const FVector2D CurrentPosition = CanvasPanelSlot->GetPosition();
		const FVector2D TargetPosition = PlayerWarningLogEachPositions[i];
		
		if (FVector2D::Distance(CurrentPosition, TargetPosition) < 0.5f)
			CanvasPanelSlot->SetPosition(TargetPosition);
		else
		{
			// 아직 멀었다면 부드럽게 이동
			const FVector2D DestPosition = FMath::Lerp(CurrentPosition, TargetPosition, 20.f * DeltaTime);
			CanvasPanelSlot->SetPosition(DestPosition);
		}
	}
	
	// 마지막에서 두 번째 -> 로그는 보이는데, FadeOut중이 아니라면 FadeOut 처리
	const int LastPrevIndex    = PlayerWarningLogSequence[PlayerWarningLogSequence.Num() - 2];
	UWidget* LastPrevLogWidget = PlayerWarningLogTexts[LastPrevIndex];

	// 아직 보이는 중인 로그에 대해, Start Fade 처리가 됐는지 확인 처리
	
	if (LastPrevLogWidget->GetRenderOpacity() != 0.f) return;	// 더 이상 보이지 않는 로그
	if (FadeOutLogs.Contains(LastPrevLogWidget)) return;		// 이미 Start Fade 처리가 된 상황
	
	if (FTimerHandle* ExistingTimer = LogLifeTimers.Find(LastPrevLogWidget))
		GetWorld()->GetTimerManager().ClearTimer(*ExistingTimer);
	
	LogLifeTimers.Remove(LastPrevLogWidget);
	StartFadeOut(LastPrevLogWidget);*/
}

void UC_InformWidget::ApplyNewLifeTimerToLog(UWidget* Log, float TotalLifeTime)
{
	// 페이드아웃 중인 위젯이었다면 취소
	FadeOutLogs.Remove(Log);
    
	// 알파값 복원 (중간에 다시 불렸을 경우를 대비)
	Log->SetRenderOpacity(1.f); 

	// 이전에 동일한 로그(위젯)에 대해 실행 중인 타이머가 있다면 취소
	if (FTimerHandle* ExistingTimer = LogLifeTimers.Find(Log))
		GetWorld()->GetTimerManager().ClearTimer(*ExistingTimer);

	FTimerDelegate LogLifeTimeExpiredDelegate{};
	LogLifeTimeExpiredDelegate.BindUFunction(this, FName("OnLogLifeTimeExpired"), Log);

	FTimerHandle NewTimerHandle{};
	GetWorld()->GetTimerManager().SetTimer(NewTimerHandle, LogLifeTimeExpiredDelegate, TotalLifeTime, false);

	LogLifeTimers.Add(Log, NewTimerHandle);
}

void UC_InformWidget::OnLogLifeTimeExpired(UWidget* TargetWidget)
{
	LogLifeTimers.Remove(TargetWidget);

	if (TargetWidget) StartFadeOut(TargetWidget);
}

void UC_InformWidget::OnInvenSlotChanged(int32 _SlotIndex, const FInventoryEntry& _ItemData)
{
	/* NOT IN USED - DEPRECATED */
	
	/*PRINT_LOCAL(GetWorld(), "InvenSlotChanged(" + _ItemData.ItemRowName.ToString() + ") : " + FString::FromInt(_SlotIndex) + " | " + FString::FromInt(_ItemData.CurCount), FColor::MakeRandomColor(), 10.f);
	PRINT_LOCAL(GetWorld(), "LockedBy : " + FString::FromInt(_ItemData.LockedByPlayerID), FColor::MakeRandomColor(), 10.f);
	PRINT_LOCAL(GetWorld(), "", FColor::MakeRandomColor(), 10.f);
	PRINT_LOCAL(GetWorld(), "", FColor::MakeRandomColor(), 10.f);*/
	
	// Inven에 아이템을 파밍을 하는 경우 -> 다음 조건을 모두 만족해야 함
	// 0. CurCount가 1 이상
	// 1. LockedBy -1(INDEX_NONE)
	// 2. Slot index가 EquippedComponent 쪽 Slot index가 아닌 경우 (0 ~ 3 x)

	if (_ItemData.CurCount <= 0) return;
	if (_ItemData.LockedByPlayerID != INDEX_NONE) return;
	if (_SlotIndex <= 3) return; // EquippedCom의 SlotIndex -> EquippedCom 장착 게임로그는 EquippedCom에서 담당할 것
	
	// Valid하게 파밍이 처리된 경우임
	const FString* pItemName = m_ItemNameMap.Find(_ItemData.ItemRowName);
	if (!pItemName)
	{
		UC_Util::Print("[UC_InformWidget::OnInvenSlotChanged] : Valid Acquired but no corresponding Item Name exists in Map", FColor::Red, 10.f);
		return;
	}
	
	AddPlayerWarningLog("ADDED TO INVENTORY : " + *pItemName + " x " + FString::FromInt(_ItemData.CurCount), FColor::White);
}
