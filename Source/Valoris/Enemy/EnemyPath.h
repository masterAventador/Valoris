// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPath.generated.h"

class USplineComponent;

/**
 * 敌人路径
 * 使用 Spline 定义敌人移动路线
 */
UCLASS()
class VALORIS_API AEnemyPath : public AActor
{
	GENERATED_BODY()

public:
	AEnemyPath();

	// 获取 Spline 组件
	UFUNCTION(BlueprintCallable, Category = "Path")
	USplineComponent* GetSplineComponent() const { return SplineComponent; }

	// 获取路径总长度
	UFUNCTION(BlueprintCallable, Category = "Path")
	float GetPathLength() const;

	// 根据距离获取位置
	UFUNCTION(BlueprintCallable, Category = "Path")
	FVector GetLocationAtDistance(float Distance) const;

	// 根据距离获取旋转
	UFUNCTION(BlueprintCallable, Category = "Path")
	FRotator GetRotationAtDistance(float Distance) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	TObjectPtr<USplineComponent> SplineComponent;
};
