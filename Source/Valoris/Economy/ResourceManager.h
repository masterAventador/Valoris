// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoldChanged, int32, NewGold, int32, Delta);

/**
 * 资源管理组件
 * 管理游戏中的金币等资源
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VALORIS_API UResourceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceManager();

	//~ 金币操作

	// 获取当前金币
	UFUNCTION(BlueprintCallable, Category = "Resource")
	int32 GetGold() const { return Gold; }

	// 增加金币
	UFUNCTION(BlueprintCallable, Category = "Resource")
	void AddGold(int32 Amount);

	// 消耗金币（如果足够）
	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool SpendGold(int32 Amount);

	// 检查是否有足够金币
	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool HasEnoughGold(int32 Amount) const { return Gold >= Amount; }

	// 设置金币（用于初始化）
	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetGold(int32 Amount);

	//~ 事件

	// 金币变化事件
	UPROPERTY(BlueprintAssignable, Category = "Resource|Events")
	FOnGoldChanged OnGoldChanged;

protected:
	// 当前金币数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	int32 Gold = 200;
};
