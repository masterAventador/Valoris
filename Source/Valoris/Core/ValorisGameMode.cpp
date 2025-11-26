// Copyright Valoris. All Rights Reserved.

#include "ValorisGameMode.h"
#include "ValorisPlayerController.h"
#include "../Camera/ValorisSpectatorPawn.h"

AValorisGameMode::AValorisGameMode()
{
	PlayerControllerClass = AValorisPlayerController::StaticClass();
	DefaultPawnClass = AValorisSpectatorPawn::StaticClass();
}

void AValorisGameMode::BeginPlay()
{
	Super::BeginPlay();
}
