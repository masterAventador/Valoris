// Copyright Valoris. All Rights Reserved.

#include "EnemyPath.h"
#include "Components/SplineComponent.h"

AEnemyPath::AEnemyPath()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建 Spline 组件
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;

	// 编辑器中始终显示 Spline
	SplineComponent->SetDrawDebug(true);

#if WITH_EDITORONLY_DATA
	// 让 Spline 在编辑器中始终可见（即使未选中）
	SplineComponent->bDrawDebug = true;
	SplineComponent->ScaleVisualizationWidth = 5.0f;
#endif
}

float AEnemyPath::GetPathLength() const
{
	if (SplineComponent)
	{
		return SplineComponent->GetSplineLength();
	}
	return 0.f;
}

FVector AEnemyPath::GetLocationAtDistance(float Distance) const
{
	if (SplineComponent)
	{
		return SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	}
	return FVector::ZeroVector;
}

FRotator AEnemyPath::GetRotationAtDistance(float Distance) const
{
	if (SplineComponent)
	{
		return SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	}
	return FRotator::ZeroRotator;
}
