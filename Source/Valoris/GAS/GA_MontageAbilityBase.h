// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ValorisGameplayAbility.h"
#include "GA_MontageAbilityBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;

/**
 * Montage 技能基类
 * 自动处理 Montage 播放和 GameplayEvent 监听
 * 子类重写 OnEventReceived() 处理事件（如伤害）
 * 默认监听所有 Event.* 事件，子类通过 Payload.EventTag 判断具体事件
 * 内置冷却系统：配置 CooldownDuration 即可自动应用冷却
 */
UCLASS(Abstract)
class VALORIS_API UGA_MontageAbilityBase : public UValorisGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MontageAbilityBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// ========== 冷却系统 ==========

	/** 返回用于检测冷却的 Tag 容器（使用技能自身的 AbilityTags） */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** 应用冷却效果 */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// ========== 动画配置 ==========

	/** 技能动画 Montage（可选，为空时跳过动画播放） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AbilityMontage;

	// ========== 冷却配置 ==========

	/** 冷却时间（秒），0 表示无冷却 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float CooldownDuration = 0.0f;

	// ========== 事件回调 ==========

	/** 收到事件时的回调（子类重写此函数处理逻辑）*/
	UFUNCTION()
	virtual void OnEventReceived(FGameplayEventData Payload);

	/** Montage 播放完成回调 */
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	/** 结束技能 */
	void EndMontageAbility(bool bWasCancelled);

	/** 获取缓存的目标（来自触发事件） */
	const AActor* GetCachedTarget() const { return CachedTarget.Get(); }

private:
	/** 缓存目标（来自触发事件） */
	TWeakObjectPtr<const AActor> CachedTarget;

	/** 缓存的冷却 Tag 容器 */
	mutable FGameplayTagContainer CooldownTagContainer;
};
