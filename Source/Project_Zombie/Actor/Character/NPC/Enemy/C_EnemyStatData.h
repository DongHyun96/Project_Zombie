#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "C_EnemyStatData.generated.h"

/// 좀비 스탯(데이터테이블)
USTRUCT(BlueprintType)
struct FC_EnemyStatData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float	MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float	MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float	DetectRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float	LoseDetectRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float	Att;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float	Def;


};