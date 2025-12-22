// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 敌人动画实例基类
 * 敌人动画相对简单，主要是前进和死亡
 */
UCLASS()
class VALORIS_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UEnemyAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 缓存的角色引用
	UPROPERTY()
	TObjectPtr<APawn> OwningPawn;

public:
	// 当前移动速度
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	// 是否在移动
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsMoving;

	// 是否死亡
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsDead;
};
