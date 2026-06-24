// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "C_EnemyStatComponent.h"

AC_BasicEnemy::AC_BasicEnemy()
{
	// 스탯 컴포넌트 추가
	m_StatCom = CreateDefaultSubobject<UC_EnemyStatComponent>(TEXT("StatComponent"));
}
