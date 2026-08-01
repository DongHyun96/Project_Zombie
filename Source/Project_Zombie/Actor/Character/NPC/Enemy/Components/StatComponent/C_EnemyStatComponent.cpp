// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyStatComponent.h"

#include "GlobalData.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utility/C_Util.h"

UC_EnemyStatComponent::UC_EnemyStatComponent()
{

}

void UC_EnemyStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 최대 속력으로 MaxWalkSpeed 조정
	if (ACharacter* OwnerZombieCharacter = Cast<ACharacter>(GetOwner()))
	{
		const float MoveSpeed = GetStat(TEXT("MoveSpeed"));

		if (MoveSpeed == 0.f)
			UC_Util::Print("From UC_EnemyStatComponent::BeginPlay : " + OwnerZombieCharacter->GetName() + "'s Move Speed is 0.", FColor::Red, 10.f);
		
		OwnerZombieCharacter->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}
	else UC_Util::Print("From UC_EnemyStatComponent::BeginPlay : Owner Actor casting to ACharacter failed!, Please attach this Comp to Zombie class", FColor::Red, 10.f);
	
	OnCurHPUpdatedDelegate.AddUObject(m_OwnerCharacter, &AC_BasicCharacter::UpdatePlayerHPOnAboveHeadTest);
}

UScriptStruct* UC_EnemyStatComponent::GetStatDataStruct() const
{
	return FEnemyStatData::StaticStruct();
}
