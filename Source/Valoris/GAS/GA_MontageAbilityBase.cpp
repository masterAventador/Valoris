// Copyright Valoris. All Rights Reserved.

#include "GA_MontageAbilityBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Effects/GE_Cooldown.h"

UGA_MontageAbilityBase::UGA_MontageAbilityBase()
{
}

const FGameplayTagContainer* UGA_MontageAbilityBase::GetCooldownTags() const
{
	// 使用技能自身的 AbilityTags 作为冷却标识
	// 这样每个技能的冷却是独立的
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags && ParentTags->Num() > 0)
	{
		return ParentTags;
	}

	// 如果父类没有设置，则使用 AbilityTags
	CooldownTagContainer = GetAssetTags();
	return &CooldownTagContainer;
}

void UGA_MontageAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CooldownDuration <= 0.0f)
	{
		return; // 无冷却
	}

	// 创建冷却效果 Spec
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UGE_Cooldown::StaticClass(), GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		return;
	}

	// 动态设置冷却时间
	SpecHandle.Data->SetDuration(CooldownDuration, true);

	// 动态添加技能的 AbilityTags 作为 GrantedTags
	// 这样冷却期间 ASC 上会有这个 Tag，技能检测到后就知道在冷却中
	SpecHandle.Data->DynamicGrantedTags.AppendTags(GetAssetTags());

	// 应用冷却效果
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
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
		EventParentTag,
		nullptr,   // OptionalExternalTarget：用技能自身 ASC
		false,     // OnlyTriggerOnce：一次激活内允许多次命中事件
		false      // OnlyMatchExact：false = 监听 Event 及其所有子 tag（Event.Attack.Hit 等）；默认 true 只精确匹配 "Event"，收不到子 tag
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
