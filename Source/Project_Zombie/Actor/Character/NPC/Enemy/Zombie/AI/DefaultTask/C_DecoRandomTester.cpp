// Fill out your copyright notice in the Description page of Project Settings.


#include "C_DecoRandomTester.h"

bool UC_DecoRandomTester::CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const
{
	return FMath::FRand() <= m_SuccessChance;
}
