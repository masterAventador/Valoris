// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ValorisAssetManager.generated.h"

/**
 * Valoris 自定义资产管理器
 * 用于在正确时机初始化 GameplayTags
 */
UCLASS()
class VALORIS_API UValorisAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	/** 获取单例 */
	static UValorisAssetManager& Get();

	/** 开始初始化加载 - 在此初始化 GameplayTags */
	virtual void StartInitialLoading() override;
};
