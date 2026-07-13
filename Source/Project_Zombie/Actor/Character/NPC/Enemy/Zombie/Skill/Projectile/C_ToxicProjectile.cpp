// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ToxicProjectile.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/ToxicAttack/C_ToxicPool.h"

AC_ToxicProjectile::AC_ToxicProjectile()
{
}

void AC_ToxicProjectile::OnHit()
{
	if (m_ToxicPoolClass)
	{
		GetWorld()->SpawnActor<AC_ToxicPool>(m_ToxicPoolClass, GetActorLocation(), GetActorRotation());
	}

	Super::OnHit();
}
