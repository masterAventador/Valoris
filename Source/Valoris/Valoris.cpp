// Copyright Epic Games, Inc. All Rights Reserved.

#include "Valoris.h"
#include "Modules/ModuleManager.h"

// GameplayTags 初始化已移至 ValorisAssetManager::StartInitialLoading()
// 以确保在 CDO 构造之前完成初始化

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Valoris, "Valoris");
