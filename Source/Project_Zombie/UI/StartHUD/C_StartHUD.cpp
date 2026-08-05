// Fill out your copyright notice in the Description page of Project Settings.


#include "C_StartHUD.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "animation/WidgetAnimation.h"

void UC_StartHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 클릭시 호출될 맴버함수 델리게이트 바인딩
	//StartBtn->OnClicked.AddDynamic(this, &UStartHUD::StartButtonClicked);
	//QuitBtn->OnClicked.AddDynamic(this, &UStartHUD::QuitButtonClicked);

	//// 버튼에 마우스가 올라가거나(Hover) 벗어날때(Unhover) 호출될 맴버함수 델리게이트 바인딩
	//StartBtn->OnHovered.AddDynamic(this, &UStartHUD::StartButtonHovered);
	//StartBtn->OnUnhovered.AddDynamic(this, &UStartHUD::StartButtonUnhovered);

	// 보유한 모든 애니메이션을 찾아낸다.
	//UWidgetBlueprintGeneratedClass* WidgetClass = GetWidgetTreeOwningClass();

	//for (int i = 0; i < WidgetClass->Animations.Num(); ++i)
	//{
	//	m_mapAnim.Add(WidgetClass->Animations[i]->GetName(), WidgetClass->Animations[i].Get());		
	//}
}

void UC_StartHUD::NativeConstruct()
{
	Super::NativeConstruct();

}

void UC_StartHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

}

void UC_StartHUD::StartButtonClicked()
{
	//UGameplayStatics::OpenLevel(GetWorld(), TEXT("TestLevel"));

	UE_LOG(LogTemp, Warning, TEXT("Start Button Clicked"));
}

void UC_StartHUD::QuitButtonClicked()
{
	/*UKismetSystemLibrary::QuitGame(GetWorld()
								 , GetWorld()->GetFirstPlayerController()
								 , EQuitPreference::Quit, true);*/

	UE_LOG(LogTemp, Warning, TEXT("Quit Button Clicked"));
}

void UC_StartHUD::StartButtonHovered()
{
	// StartBtn 이 1.2배로 커지는 위젯 애니메이션 재생
	/*UWidgetAnimation* pAnim = m_mapAnim.FindRef(TEXT("OnHoverScaleUp_INST"));
	if (pAnim)
		PlayAnimation(pAnim);*/


		//PlayAnimation(OnHoverScaleUp);
}

void UC_StartHUD::StartButtonUnhovered()
{
	// StartBtn 이 1.2배에서 다기 1배로 작아지는 위젯 애니메이션 재생
	/*UWidgetAnimation* pAnim = m_mapAnim.FindRef(TEXT("UnhoverScaleDown_INST"));
	if (pAnim)
		PlayAnimation(pAnim);*/

		//PlayAnimation(UnhoverScaleDown);
}
