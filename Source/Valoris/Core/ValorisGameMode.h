// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValorisGameMode.generated.h"

/**
 * 游戏模式基类
 * 负责游戏流程控制、胜负判定
 */
UCLASS()
class VALORIS_API AValorisGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AValorisGameMode();

protected:
	virtual void BeginPlay() override;
};
