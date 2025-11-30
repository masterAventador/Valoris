// Copyright Valoris. All Rights Reserved.

#include "ValorisGameplayAbility.h"

UValorisGameplayAbility::UValorisGameplayAbility()
{
	// 默认设置：本地预测，服务器执行
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
