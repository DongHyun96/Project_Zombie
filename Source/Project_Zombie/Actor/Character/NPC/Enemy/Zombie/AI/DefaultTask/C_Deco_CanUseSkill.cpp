// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Deco_CanUseSkill.h"

#include "AIController.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"

UC_Deco_CanUseSkill::UC_Deco_CanUseSkill()
{
    NodeName = TEXT("Can Use Skill");
}

bool UC_Deco_CanUseSkill::CalculateRawConditionValue(UBehaviorTreeComponent& _OwnerCom, uint8* _NodeMemory) const
{
    AAIController* Controller = _OwnerCom.GetAIOwner();

    if (!Controller)
        return false;

    AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(Controller->GetPawn());

    if (!Enemy)
        return false;

    UC_EnemySkillComponent* SkillCom = Enemy->FindComponentByClass<UC_EnemySkillComponent>();

    if (!SkillCom)
        return false;

    return SkillCom->CanUseSkill(m_SkillSlot);
}
