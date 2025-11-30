// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HeroAIController.generated.h"

class AValorisCharacterBase;

/**
 * 英雄 AI 控制器
 * 负责处理英雄的导航移动和自动攻击
 */
UCLASS()
class VALORIS_API AHeroAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHeroAIController();

	virtual void Tick(float DeltaTime) override;

	// 移动到目标位置
	void MoveToTargetLocation(const FVector& TargetLocation);

	// 设置攻击目标
	void SetAttackTarget(AValorisCharacterBase* Target);

	// 清除攻击目标
	void ClearAttackTarget();

	// 获取攻击目标
	AValorisCharacterBase* GetAttackTarget() const { return AttackTarget; }

protected:
	// 移动完成回调
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	// 检查是否在攻击范围内
	bool IsInAttackRange() const;

	// 执行攻击
	void PerformAttack();

	// 追击目标
	void ChaseTarget();

	// 当前攻击目标
	UPROPERTY()
	TObjectPtr<AValorisCharacterBase> AttackTarget;

	// 攻击冷却计时器
	float AttackCooldownTimer = 0.f;

	// 是否正在追击
	bool bIsChasing = false;
};
