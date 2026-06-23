// Copyright Valoris. All Rights Reserved.

#include "ExecCalc_Damage.h"
#include "ValorisAttributeSet.h"
#include "ValorisGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "../Combat/ArenaCombatUtils.h"

// 声明要捕获的属性
struct FDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FDamageStatics()
	{
		// 捕获目标的防御力
		DEFINE_ATTRIBUTE_CAPTUREDEF(UValorisAttributeSet, Defense, Target, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 目标处于无敌（闪避无敌帧）则不结算任何伤害
	if (EvaluateParams.TargetTags && EvaluateParams.TargetTags->HasTag(FValorisGameplayTags::State_Invincible))
	{
		return;
	}

	// 从 SetByCaller 获取基础伤害（已经在 GA 里算好了 AttackPower * DamageMultiplier）
	float BaseDamage = Spec.GetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, false, 0.f);

	// 获取目标的防御力
	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvaluateParams, Defense);

	// 计算最终伤害：基础伤害 - 防御力（最低为1）
	float FinalDamage = ArenaCombat::ComputeMitigatedDamage(BaseDamage, Defense);

	// 输出到 IncomingDamage 属性
	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UValorisAttributeSet::GetIncomingDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage
			)
		);
	}
}
