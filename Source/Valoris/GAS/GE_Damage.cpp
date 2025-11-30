// Copyright Valoris. All Rights Reserved.

#include "GE_Damage.h"
#include "MMC_Damage.h"

UGE_Damage::UGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// 添加执行计算：由 ExecCalc_Damage 计算伤害并应用
	FGameplayEffectExecutionDefinition ExecutionDef;
	ExecutionDef.CalculationClass = UExecCalc_Damage::StaticClass();

	Executions.Add(ExecutionDef);
}
