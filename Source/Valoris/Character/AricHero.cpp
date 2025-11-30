// Copyright Valoris. All Rights Reserved.

#include "AricHero.h"
#include "HeroAIController.h"

AAricHero::AAricHero()
{
	// 英雄使用 HeroAIController
	AIControllerClass = AHeroAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
