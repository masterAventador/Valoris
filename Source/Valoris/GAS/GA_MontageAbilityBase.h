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
 */
UCLASS(Abstract)
class VALORIS_API UGA_MontageAbilityBase : public UValorisGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MontageAbilityBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 技能动画 Montage
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AbilityMontage;

	// 收到事件时的回调（子类重写此函数处理逻辑）
	// Payload.EventTag 可用于判断具体是什么事件
	UFUNCTION()
	virtual void OnEventReceived(FGameplayEventData Payload);

	// Montage 播放完成回调
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	// 结束技能
	void EndMontageAbility(bool bWasCancelled);

	// 获取缓存的目标（来自触发事件）
	const AActor* GetCachedTarget() const { return CachedTarget.Get(); }

private:
	// 缓存目标（来自触发事件）
	TWeakObjectPtr<const AActor> CachedTarget;
};
