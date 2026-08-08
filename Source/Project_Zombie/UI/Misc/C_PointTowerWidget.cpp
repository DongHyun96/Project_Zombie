#include "UI/Misc/C_PointTowerWidget.h"

#include "Components/TextBlock.h"

void UC_PointTowerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	m_OriginColor   = ConqueredPercentText->GetColorAndOpacity();
	m_LerpDestColor = m_OriginColor;
}

void UC_PointTowerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// FMath::Lerp(m_OriginColor, m_LerpDestColor, 0.5f);
	
	m_OriginColor = FMath::Lerp
	(
		m_OriginColor.GetSpecifiedColor(),
		m_LerpDestColor.GetSpecifiedColor(), // 언제나 원상태의 Color로 복귀하도록
		1.5f * InDeltaTime
	);
	
	ConqueredPercentText->SetColorAndOpacity(m_OriginColor);
}

void UC_PointTowerWidget::SetPercentText(uint8 _Percent)
{
	ConqueredPercentText->SetText(
		FText::Format(
			NSLOCTEXT("PointTower", "ConqueredPercent", "Conquered {0}%"),
			FText::AsNumber(_Percent)
		)
	);
}

void UC_PointTowerWidget::OnDamaged()
{
	m_OriginColor = m_DamagedColor;
}