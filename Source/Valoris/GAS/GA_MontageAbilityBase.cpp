// Copyright Valoris. All Rights Reserved.

#include "GA_MontageAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_MontageAbilityBase::UGA_MontageAbilityBase()
{
}

void UGA_MontageAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 缓存目标（来自触发事件）
	if (TriggerEventData && TriggerEventData->Target)
	{
		CachedTarget = TriggerEventData->Target;
	}

	// 监听所有 Event.* 事件（使用父级 Tag）
	// 子类在 OnEventReceived 中通过 Payload.EventTag 判断具体事件
	FGameplayTag EventParentTag = FGameplayTag::RequestGameplayTag(FName("Event"));
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		EventParentTag
	);
	WaitEventTask->EventReceived.AddDynamic(this, &UGA_MontageAbilityBase::OnEventReceived);
	WaitEventTask->ReadyForActivation();

	// 播放动画
	if (AbilityMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			AbilityMontage,
			1.0f,
			NAME_None,
			true,
			1.0f
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_MontageAbilityBase::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_MontageAbilityBase::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_MontageAbilityBase::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_MontageAbilityBase::OnMontageCancelled);

		MontageTask->ReadyForActivation();
	}
	else
	{
		// 没有 Montage，直接结束
		EndMontageAbility(false);
	}
}

void UGA_MontageAbilityBase::OnEventReceived(FGameplayEventData Payload)
{
	// 基类空实现，子类重写处理逻辑
	// 可通过 Payload.EventTag 判断具体事件类型
}

void UGA_MontageAbilityBase::OnMontageCompleted()
{
	EndMontageAbility(false);
}

void UGA_MontageAbilityBase::OnMontageCancelled()
{
	EndMontageAbility(true);
}

void UGA_MontageAbilityBase::EndMontageAbility(bool bWasCancelled)
{
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), true, bWasCancelled);
}
