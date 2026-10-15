// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointIndicatorManagerWidget.h"

#include "C_PointIndicatorWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/PointTower/C_PointTower.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/C_Util.h"

void UC_PointIndicatorManagerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	m_PointIndicatorWidgetPool.Empty();

	for (int32 i = 0; i < 4; ++i) // 한 Seq 내의 PointTower가 4개보다 많아지면(그럴 가능성은 매우 없긴 함), 개수 조정해줄 것
	{
		const FName PIndicatorWidgetName    = *FString::Printf(TEXT("PointIndicator%d"), i);
		UC_PointIndicatorWidget* PIndicator = Cast<UC_PointIndicatorWidget>(GetWidgetFromName(PIndicatorWidgetName));
		
		if (!PIndicator)
		{
			UC_Util::Print("[UC_PointIndicatorManagerWidget::NativeOnInitialized] : Point Indicator Widget nullptr!", FColor::Red, 10.f);
			continue;
		}

		m_PointIndicatorWidgetPool.Add(PIndicator);
	}


	m_IndicatorSample = m_PointIndicatorWidgetPool.Top();
	
	// ClampMin, ClampMax 값 지정
	if (m_PointIndicatorWidgetPool.IsEmpty())
	{
		UC_Util::Print("[UC_PointIndicatorManagerWidget::NativeOnInitialized] : Point Indicator Widget pool not initialized!", FColor::Red, 10.f);
		return;
	}

	FVector2D IndicatorSize{};

	// 모두 동일한 크기의 Indicator위젯
	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(m_PointIndicatorWidgetPool[0]))
	   IndicatorSize = CanvasSlot->GetSize();

	UC_Util::Print("IndicatorSize : " + IndicatorSize.ToString(), FColor::MakeRandomColor(), 10.f);

	m_ClampMin       = IndicatorSize * 0.5f;
	m_ClampMax       = FVector2D(1920.f, 1080.f) - IndicatorSize * 0.5f;
	m_ScreenMiddle2D = FVector2D(1920.f, 1080.f) * 0.5f;
}

void UC_PointIndicatorManagerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_PointIndicatorManagerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (m_ActivePointIndicators.IsEmpty()) return;

	UpdateViewportInfo();

	for (const TTuple<AC_PointTower*, UC_PointIndicatorWidget*>& ActivePointIndicator : m_ActivePointIndicators)
	{
		if (!ActivePointIndicator.Key)
		{
			UC_Util::Print("[UC_PointIndicatorManagerWidget::NativeTick] : PointTower nullptr", FColor::Red, 10.f);
			continue;
		}

		AC_PointTower* PointTower                = ActivePointIndicator.Key;
		UC_PointIndicatorWidget* IndicatorWidget = ActivePointIndicator.Value;

		const FVector ObjectLocation = PointTower->GetGenerator()->GetComponentLocation();

		// 현재 표기용 WholeOutline이 모두 켜져 있지 않은 상황(플레이어가 해당 PointTower에 충분히 가까운 상황에서는 Indicator 표기 처리하지 않는다)
		if (!PointTower->IsGeneratorOutlineActive())
		{
			if (IndicatorWidget->IsCurrentlyVisible())
				IndicatorWidget->SetVisibility(ESlateVisibility::Collapsed);
			
			continue;
		}

		FVector2D ProjectedLocation{};
		const bool bOnScreen = IsWorldLocationWithinScreenClamp(ObjectLocation, ProjectedLocation);

		if (bOnScreen) // 스크린에 잡힘 (Player 화면에 PointTower가 보이는 중)
		{
			// 해당 PointTower가 직접 보이는 경우에는 따로 표기하지 않음

			if (IndicatorWidget->IsCurrentlyVisible())
				IndicatorWidget->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		// 화면에 PointTower가 잡히지 않는 중 -> 이 때에는 Indicator 표기 처리를 해준다
		if (!IndicatorWidget->IsCurrentlyVisible())
			IndicatorWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// Indicator 위젯 위치와 삼각형 방향 회전 잡기처리
		CalculateOffScreenLocation(ObjectLocation, IndicatorWidget);
	}
}

bool UC_PointIndicatorManagerWidget::RegisterPointTowerIndicator(AC_PointTower* _PointTower)
{
	if (m_ActivePointIndicators.Contains(_PointTower)) return false;
	if (m_PointIndicatorWidgetPool.IsEmpty()) return false;

	// 등록 처리
	UC_PointIndicatorWidget* IndicatorWidget = m_PointIndicatorWidgetPool.Pop();
	m_ActivePointIndicators.Add(_PointTower, IndicatorWidget);

	IndicatorWidget->SetPointTower(_PointTower);

	return true;
}

bool UC_PointIndicatorManagerWidget::DeRegisterPointTowerIndicator(AC_PointTower* _PointTower)
{
	if (!m_ActivePointIndicators.Contains(_PointTower)) return false;

	// 등록 해제 처리
	UC_PointIndicatorWidget* TargetWidget = m_ActivePointIndicators[_PointTower];
	TargetWidget->SetPointTower(nullptr);
	m_ActivePointIndicators.Remove(_PointTower);
	m_PointIndicatorWidgetPool.Add(TargetWidget);

	return true;
}

void UC_PointIndicatorManagerWidget::UpdateViewportInfo()
{
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);

	if (m_bInitializedViewport && ViewportSize == m_SavedViewportSize) return;

	m_bInitializedViewport = true;
	m_SavedViewportSize    = ViewportSize;
	
	// const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);

	/*-> SlotAsCanvasSlot
	-> GetSize
	-> / 2
	-> Clamp Min
   즉 Indicator 크기의 절반을 최소 Clamp 여백으로 사용*/

	/*FVector2D IndicatorSize = FVector2D::ZeroVector;

	// 모두 동일한 크기의 Indicator위젯
	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(m_IndicatorSample))
	   IndicatorSize = CanvasSlot->GetSize();*/

	/*m_ClampMin       = (IndicatorSize * 0.5f) / ViewportScale;
	m_ClampMax       = (ViewportSize / ViewportScale) - m_ClampMin;
	m_ScreenMiddle2D = (ViewportSize / ViewportScale) * 0.5f;*/
}

bool UC_PointIndicatorManagerWidget::IsWorldLocationWithinScreenClamp(const FVector& _WorldLocation, FVector2D& _OutScreenLocation) const
{
	APlayerController* PlayerController = GetOwningPlayer();

	if (!PlayerController)
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		UC_Util::Print
		(
			"[UC_PointIndicatorManagerWidget::IsWorldLocationWithinScreenClamp] : PlayerController nullptr!",
			FColor::Red,
			10.f
		);

		return false;
	}

	FVector2D ScreenLocation{};

	if (!UGameplayStatics::ProjectWorldToScreen(PlayerController, _WorldLocation, ScreenLocation, true))
		return false;

	_OutScreenLocation = ScreenLocation;

	return	ScreenLocation.X >= m_ClampMin.X &&
			ScreenLocation.X <= m_ClampMax.X &&
			ScreenLocation.Y >= m_ClampMin.Y &&
			ScreenLocation.Y <= m_ClampMax.Y;
}

bool UC_PointIndicatorManagerWidget::CalculateOffScreenLocation(const FVector& _ObjectLocation, UC_PointIndicatorWidget* _Widget)
{
	APlayerController* PlayerController = GetOwningPlayer();

	if (!PlayerController)
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		UC_Util::Print
		(
			"[UC_PointIndicatorManagerWidget::CalculateOffScreenLocation] : PlayerController nullptr!",
			FColor::Red,
			10.f
		);

		return false;
	}


	/* Viewport 중앙을 World로 Deproject */
	const FVector2D ViewportMiddle = m_SavedViewportSize * 0.5f;

	FVector WorldPosition{};
	FVector WorldDirection{};

	if (!UGameplayStatics::DeprojectScreenToWorld(PlayerController, ViewportMiddle, WorldPosition, WorldDirection))
		return false;

	/* Object 방향 계산 */

	FVector ObjectDirection = (_ObjectLocation - WorldPosition).GetSafeNormal();

	/*MiddlePointTowardObject
	Middle Point + Object Direction * Accuracy*/

	const FVector MidPointTowardObject = WorldPosition + ObjectDirection * m_Accuracy;

	// 이 위치가 다시 화면 Clamp 내부인지 검사 (이거 빼줘도 될거같기도 하고)
	FVector2D DummyScreenLocation{};

	if (!IsWorldLocationWithinScreenClamp(MidPointTowardObject, DummyScreenLocation))
	{
		PRINT_LOCAL(GetWorld(), "[UC_PointIndicatorManagerWidget::CalculateOffScreenLocation] : POINT NOT ON SCREEN",
					FColor::Red, 10.f);
		return false;
	}

	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (!CameraManager) return false;

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FRotator CameraRotation = CameraManager->GetCameraRotation();

	 /* Camera -> Object World 방향 */
	const FVector CameraToObjectDirection = (_ObjectLocation - CameraLocation).GetSafeNormal();
	if (CameraToObjectDirection.IsNearlyZero()) return false;

	/* Camera의 Screen Right / Up 방향 */
	const FVector CameraForward = CameraRotation.RotateVector(FVector::ForwardVector);
	const FVector CameraRight   = CameraRotation.RotateVector(FVector::RightVector);
	const FVector CameraUp      = CameraRotation.RotateVector(FVector::UpVector);

	/*
	 * World 방향을 Screen 방향으로 변환
	 *
	 * X : 화면 오른쪽
	 * Y : 화면 아래쪽
	 *
	 * Unreal의 Up은 Screen Y와 반대
	 */
	FVector2D ScreenDirection{};

	const float ForwardDot = FVector::DotProduct(CameraForward, CameraToObjectDirection);
	
	/*
	 * PointTower가 카메라 뒤쪽에 있는 경우
	 *
	 * 카메라의 Pitch에 관계없이
	 * Indicator는 항상 화면 아래쪽을 향하게 한다.
	 */
	if (ForwardDot >= 0.f)
	{
		ScreenDirection.X = FVector::DotProduct(CameraToObjectDirection, CameraRight);
		ScreenDirection.Y = -FVector::DotProduct(CameraToObjectDirection, CameraUp);

		if (ScreenDirection.IsNearlyZero()) ScreenDirection = FVector2D(0.f, 1.f);
		else							    ScreenDirection.Normalize();
	}
	else // 등진 경우 (PointTower가 카메라 뒤쪽에 있는 경우) -> X 방향은 PointTower의 실제 좌우 방향을 유지, Y방향만 화면 아래쪽으로 고정 처리
	{
		ScreenDirection.X = FVector::DotProduct(CameraToObjectDirection, CameraRight);
		ScreenDirection.Y = 1.f;
		ScreenDirection.Normalize();
	}

	FVector2D WidgetScreenPos  = m_ScreenMiddle2D + ScreenDirection;;
	const FVector2D RunAndRise = m_ScreenMiddle2D - WidgetScreenPos;

	/* 
	 * 최소로 필요한 Rise / Run 계산
	 * (ScreenMiddle - ClampMin) / RunAndRise  
	 */
	const FVector2D RequiredLength    = (m_ScreenMiddle2D - m_ClampMin) / RunAndRise;
	const FVector2D AbsRequiredLength = RequiredLength.GetAbs();
	const float LineLength            = AbsRequiredLength.GetMin();

	/* 화면 Edge 위치 계산 */
	WidgetScreenPos = m_ScreenMiddle2D - RunAndRise * LineLength;

	UpdateWidgetLocation(_Widget, WidgetScreenPos);
	UpdateIndicatorAngle(_Widget, WidgetScreenPos);

	return true;
}

void UC_PointIndicatorManagerWidget::UpdateWidgetLocation(UC_PointIndicatorWidget* _Widget, FVector2D _WidgetScreenPos)
{
	// WidgetScreenLocation을 Clamp Min ~ Clamp Max 사이로 제한

	_WidgetScreenPos.X = FMath::Clamp(_WidgetScreenPos.X, m_ClampMin.X, m_ClampMax.X);
	_WidgetScreenPos.Y = FMath::Clamp(_WidgetScreenPos.Y, m_ClampMin.Y, m_ClampMax.Y);

	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(_Widget))
		CanvasSlot->SetPosition(_WidgetScreenPos);
}

void UC_PointIndicatorManagerWidget::UpdateIndicatorAngle(UC_PointIndicatorWidget* _Widget, const FVector2D& _WidgetScreenPos)
{
	const float SquareMin = FMath::Min(m_ClampMin.X, m_ClampMin.Y);
	const float SquareMax = FMath::Min(m_ClampMax.X, m_ClampMax.Y);

	// X/Y를 각각 SquareMin ~ SquareMax 범위로 Normalize
	const double NormalizedX = FMath::GetMappedRangeValueClamped
	(
		FVector2D(m_ClampMin.X, m_ClampMax.X),
		FVector2D(0.f, 1.f),
		_WidgetScreenPos.X
	);

	const double NormalizedY = FMath::GetMappedRangeValueClamped
	(
		FVector2D(m_ClampMin.Y, m_ClampMax.Y),
		FVector2D(0.f, 1.f),
		_WidgetScreenPos.Y
	);

	const double SquareX = FMath::Lerp(SquareMin, SquareMax, NormalizedX);
	const double SquareY = FMath::Lerp(SquareMin, SquareMax, NormalizedY);

	// Screen Middle도 Square로 변환
	const float SquareMiddle = FMath::Min(m_ScreenMiddle2D.X, m_ScreenMiddle2D.Y);

	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation
	(
		FVector(SquareMiddle, SquareMiddle, 0.f),
		FVector(SquareX, SquareY, 0.f)
	);

	const float Angle = LookAtRotation.Yaw + 90.f;
	_Widget->SetTriangleImgAngle(Angle);
}
