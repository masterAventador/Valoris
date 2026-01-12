// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Cooldown.generated.h"

/**
 * 通用冷却效果
 * 纯 C++ 实现，不需要蓝图配置
 * Duration 和 GrantedTags 由技能动态设置
 */
UCLASS()
class VALORIS_API UGE_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Cooldown();
};
