// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HeroAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * 英雄动画实例基类
 * 提供通用的动画变量（Speed、Direction等）
 * 子类蓝图继承此类，替换动画资源即可
 */
UCLASS()
class VALORIS_API UHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UHeroAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 缓存的角色引用
	UPROPERTY()
	TObjectPtr<ACharacter> OwningCharacter;

	// 缓存的移动组件引用
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

public:
	// 当前移动速度（用于 Blend Space）
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	// 移动方向（-180 到 180，用于 8 方向 Blend Space）
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Direction;

	// 是否在移动
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsMoving;

	// 是否在空中
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsInAir;

	// 是否死亡
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsDead;
};
