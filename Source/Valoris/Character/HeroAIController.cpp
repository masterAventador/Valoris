// Copyright Valoris. All Rights Reserved.

#include "HeroAIController.h"
#include "Navigation/PathFollowingComponent.h"

void AHeroAIController::MoveToTargetLocation(const FVector& TargetLocation)
{
	MoveToLocation(TargetLocation, 50.f, true, true, false, true);
}

void AHeroAIController::StopMovement()
{
	Super::StopMovement();
}
