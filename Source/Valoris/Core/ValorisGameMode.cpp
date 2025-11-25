// Copyright Valoris. All Rights Reserved.

#include "ValorisGameMode.h"
#include "ValorisPlayerController.h"

AValorisGameMode::AValorisGameMode()
{
	PlayerControllerClass = AValorisPlayerController::StaticClass();
}

void AValorisGameMode::BeginPlay()
{
	Super::BeginPlay();
}
