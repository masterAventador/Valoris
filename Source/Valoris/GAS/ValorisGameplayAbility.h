// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ValorisGameplayAbility.generated.h"

/**
 * 技能基类
 * 所有 Valoris 技能都继承自此类
 */
UCLASS()
class VALORIS_API UValorisGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UValorisGameplayAbility();
};
