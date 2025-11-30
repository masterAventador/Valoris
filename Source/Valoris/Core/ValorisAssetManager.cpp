// Copyright Valoris. All Rights Reserved.

#include "ValorisAssetManager.h"
#include "../GAS/ValorisGameplayTags.h"

UValorisAssetManager& UValorisAssetManager::Get()
{
	check(GEngine);

	UValorisAssetManager* AssetManager = Cast<UValorisAssetManager>(GEngine->AssetManager);
	if (AssetManager)
	{
		return *AssetManager;
	}

	// 如果没有配置自定义 AssetManager，返回默认的（但这不应该发生）
	UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManager class in DefaultEngine.ini. Must be set to ValorisAssetManager!"));

	// 永远不会执行到这里，但编译器需要返回值
	return *NewObject<UValorisAssetManager>();
}

void UValorisAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// 在资产加载开始时初始化 GameplayTags
	// 这个时机早于大多数 CDO 构造，是初始化 Native Tags 的最佳时机
	FValorisGameplayTags::InitializeNativeTags();
}
