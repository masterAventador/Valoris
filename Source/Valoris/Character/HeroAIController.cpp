// Copyright Valoris. All Rights Reserved.

#include "HeroAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AbilitySystemComponent.h"
#include "ValorisCharacterBase.h"
#include "../GAS/ValorisAttributeSet.h"
#include "../GAS/ValorisGameplayTags.h"

AHeroAIController::AHeroAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHeroAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 如果有攻击目标
	if (AttackTarget && IsValid(AttackTarget))
	{
		if (IsInAttackRange())
		{
			// 在攻击范围内，停止移动并攻击
			StopMovement();
			bIsChasing = false;

			// 尝试攻击（冷却由 GAS 管理）
			PerformAttack();
		}
		else if (!bIsChasing)
		{
			// 不在攻击范围内，追击
			ChaseTarget();
		}
	}
}

void AHeroAIController::MoveToTargetLocation(const FVector& TargetLocation)
{
	bIsChasing = false;
	MoveToLocation(TargetLocation, 50.f, true, true, false, true);
}

void AHeroAIController::SetAttackTarget(AValorisCharacterBase* Target)
{
	AttackTarget = Target;
	if (Target)
	{
		ChaseTarget();
	}
}

void AHeroAIController::ClearAttackTarget()
{
	AttackTarget = nullptr;
	bIsChasing = false;
}

void AHeroAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	bIsChasing = false;

	// 如果有攻击目标且不在范围内，继续追击
	if (AttackTarget && IsValid(AttackTarget) && !IsInAttackRange())
	{
		ChaseTarget();
	}
}

bool AHeroAIController::IsInAttackRange() const
{
	if (!AttackTarget || !GetPawn())
	{
		return false;
	}

	// 从属性集获取攻击范围
	float AttackRange = 200.f; // 默认值
	if (AValorisCharacterBase* Hero = Cast<AValorisCharacterBase>(GetPawn()))
	{
		if (UValorisAttributeSet* Attributes = Hero->GetAttributeSet())
		{
			AttackRange = Attributes->GetAttackRange();
		}
	}

	float Distance = FVector::Dist(GetPawn()->GetActorLocation(), AttackTarget->GetActorLocation());
	return Distance <= AttackRange;
}

void AHeroAIController::PerformAttack()
{
	if (!AttackTarget || !GetPawn())
	{
		return;
	}

	AValorisCharacterBase* Hero = Cast<AValorisCharacterBase>(GetPawn());
	if (!Hero)
	{
		return;
	}

	// 通过 GAS 触发攻击技能（冷却由 GA 内部管理）
	UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent();
	if (ASC)
	{
		// 构造事件数据，传入攻击目标
		FGameplayEventData EventData;
		EventData.Target = AttackTarget;

		// 触发攻击事件
		ASC->HandleGameplayEvent(FValorisGameplayTags::Event_Attack, &EventData);
	}
}

void AHeroAIController::ChaseTarget()
{
	if (!AttackTarget)
	{
		return;
	}

	bIsChasing = true;

	// 获取攻击范围，追到攻击范围边缘就停
	float AttackRange = 150.f;
	if (AValorisCharacterBase* Hero = Cast<AValorisCharacterBase>(GetPawn()))
	{
		if (UValorisAttributeSet* Attributes = Hero->GetAttributeSet())
		{
			AttackRange = Attributes->GetAttackRange();
		}
	}

	// 追击到攻击范围内（稍微近一点以确保能攻击到）
	float AcceptanceRadius = FMath::Max(50.f, AttackRange - 50.f);
	MoveToActor(AttackTarget, AcceptanceRadius, true, true, true);
}
