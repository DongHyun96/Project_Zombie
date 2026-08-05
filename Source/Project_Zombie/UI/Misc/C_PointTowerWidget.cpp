#include "UI/Misc/C_PointTowerWidget.h"

#include "Components/TextBlock.h"

void UC_PointTowerWidget::SetPercentText(uint8 _Percent)
{
	ConqueredPercentText->SetText(
		FText::Format(
			NSLOCTEXT("PointTower", "ConqueredPercent", "Conquered {0}%"),
			FText::AsNumber(_Percent)
		)
	);
}
