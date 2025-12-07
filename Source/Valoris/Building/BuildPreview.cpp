// Copyright Valoris. All Rights Reserved.

#include "BuildPreview.h"
#include "../Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetSystemLibrary.h"

ABuildPreview::ABuildPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建根组件
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 创建预览网格
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 默认使用一个简单的圆柱体作为预览
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(CylinderMesh.Object);
		PreviewMesh->SetWorldScale3D(FVector(1.f, 1.f, 0.5f));
	}
}

void ABuildPreview::SetTowerClass(TSubclassOf<ATowerBase> InTowerClass)
{
	CurrentTowerClass = InTowerClass;

	// TODO: 可以根据塔类型设置不同的预览网格
}

void ABuildPreview::UpdatePreviewLocation(const FVector& Location)
{
	SetActorLocation(Location);

	// 检查是否可建造
	bCanBuild = !CheckOverlap();

	// 更新材质颜色
	UpdatePreviewMaterial();
}

bool ABuildPreview::CanBuildAtCurrentLocation() const
{
	return bCanBuild;
}

void ABuildPreview::SetPreviewVisible(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
}

void ABuildPreview::UpdatePreviewMaterial()
{
	if (PreviewMesh)
	{
		UMaterialInterface* Material = bCanBuild ? ValidMaterial : InvalidMaterial;
		if (Material)
		{
			PreviewMesh->SetMaterial(0, Material);
		}
	}
}

bool ABuildPreview::CheckOverlap() const
{
	// 检测当前位置是否有重叠物体
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(const_cast<ABuildPreview*>(this));

	TArray<AActor*> OverlappingActors;

	bool bHasOverlap = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		OverlapCheckRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(), // 检测所有类型
		ATowerBase::StaticClass(), // 只检测塔
		IgnoredActors,
		OverlappingActors
	);

	return bHasOverlap;
}
