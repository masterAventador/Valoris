// Copyright Valoris. All Rights Reserved.

#include "HeroAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "ValorisCharacterBase.h"
#include "../GAS/ValorisAttributeSet.h"

AHeroAIController::AHeroAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHeroAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新攻击冷却
	if (AttackCooldownTimer > 0.f)
	{
		AttackCooldownTimer -= DeltaTime;
	}

	// 如果有攻击目标
	if (AttackTarget && IsValid(AttackTarget))
	{
		if (IsInAttackRange())
		{
			// 在攻击范围内，停止移动并攻击
			StopMovement();
			bIsChasing = false;

			if (AttackCooldownTimer <= 0.f)
			{
				PerformAttack();
			}
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

	// 获取攻击速度来计算冷却时间
	float AttackSpeed = 1.f;
	if (UValorisAttributeSet* Attributes = Hero->GetAttributeSet())
	{
		AttackSpeed = FMath::Max(0.1f, Attributes->GetAttackSpeed());
	}
	AttackCooldownTimer = 1.f / AttackSpeed;

	// TODO: 通过 GAS 触发攻击技能
	// 现在先用日志占位
	UE_LOG(LogTemp, Warning, TEXT("%s attacks %s"), *Hero->GetName(), *AttackTarget->GetName());
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
