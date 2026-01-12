// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Buff.generated.h"

/**
 * 通用增益效果基类
 * 用于提供临时属性加成
 * 子类/蓝图配置具体的属性修改
 */
UCLASS()
class VALORIS_API UGE_Buff : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Buff();
};
