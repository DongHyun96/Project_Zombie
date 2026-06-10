// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "C_Util.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Util : public UObject
{
	GENERATED_BODY()
	
public:

	static void Print(const FString&		str,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(int					data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(float					data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(double				data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(const FVector&		data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(const FVector2D&		data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(const FTransform&		data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(const FRotator&		data,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);
	static void Print(AActor*				Actor,	const FColor& InColor = FColor::Red, float TimeToDisplay = 1.f);

	static void PrintLogMessage(const FString& str);

	static void ClearAllMessages() { GEngine->ClearOnScreenDebugMessages(); }

public:
	/// <summary>
	/// FVector XY 성분 떼어서 FVector2D return
	/// </summary>
	static FVector2D GetXY(const FVector& InVector) { return FVector2D(InVector.X, InVector.Y); }

};
