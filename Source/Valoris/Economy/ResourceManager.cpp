// Copyright Valoris. All Rights Reserved.

#include "ResourceManager.h"

UResourceManager::UResourceManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UResourceManager::AddGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	Gold += Amount;
	OnGoldChanged.Broadcast(Gold, Amount);

	UE_LOG(LogTemp, Log, TEXT("ResourceManager: +%d Gold, Total: %d"), Amount, Gold);
}

bool UResourceManager::SpendGold(int32 Amount)
{
	if (Amount <= 0 || !HasEnoughGold(Amount))
	{
		return false;
	}

	Gold -= Amount;
	OnGoldChanged.Broadcast(Gold, -Amount);

	UE_LOG(LogTemp, Log, TEXT("ResourceManager: -%d Gold, Total: %d"), Amount, Gold);
	return true;
}

void UResourceManager::SetGold(int32 Amount)
{
	int32 Delta = Amount - Gold;
	Gold = FMath::Max(0, Amount);

	if (Delta != 0)
	{
		OnGoldChanged.Broadcast(Gold, Delta);
	}
}
