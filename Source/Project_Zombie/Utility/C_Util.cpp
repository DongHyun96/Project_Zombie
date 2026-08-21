// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Util.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

void UC_Util::Print(const FString& str, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *str);
#endif
}

void UC_Util::Print(int data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	FString str = FString::FromInt(data);
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *str);
#endif
}

void UC_Util::Print(float data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	FString str = FString::SanitizeFloat(data);
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *str);
#endif
}

void UC_Util::Print(double data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	FString str = FString::SanitizeFloat(data);
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *str);
#endif
}

void UC_Util::Print(const FVector& data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	FString str = data.ToString();
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *str);
#endif
}

void UC_Util::Print(const FVector2D& data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	FString str = data.ToString();
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *str);
#endif
}

void UC_Util::Print(const FTransform& data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *data.ToString());
#endif
}

void UC_Util::Print(const FRotator& data, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, *data.ToString());
#endif
}

void UC_Util::Print(AActor* Actor, const FColor& InColor, float TimeToDisplay)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	FString AddressString = FString::Printf(TEXT("%p"), Actor);
	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, InColor, AddressString);
#endif
}

void UC_Util::PrintLogMessage(const FString& str)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	UE_LOG(LogTemp, Log, TEXT("%s"), *str);
#endif
	
}
