// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage.generated.h"

/**
 * 伤害效果基类
 * 使用 MMC_Damage 计算伤害值，然后应用到 IncomingDamage 属性
 */
UCLASS()
class VALORIS_API UGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Damage();
};
