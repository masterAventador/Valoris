// Copyright Valoris. All Rights Reserved.

#include "ExecCalc_Damage.h"
#include "ValorisAttributeSet.h"
#include "AbilitySystemComponent.h"

// 声明要捕获的属性
struct FDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	FDamageStatics()
	{
		// 捕获攻击者的攻击力
		DEFINE_ATTRIBUTE_CAPTUREDEF(UValorisAttributeSet, AttackPower, Source, true);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 获取攻击者的攻击力
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluateParams, AttackPower);

	// 计算最终伤害（后续可扩展：减去护甲、暴击等）
	float FinalDamage = FMath::Max(0.f, AttackPower);

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
