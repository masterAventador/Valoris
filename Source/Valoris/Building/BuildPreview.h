// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildPreview.generated.h"

class ATowerBase;

/**
 * 建造预览
 * 显示塔的放置位置，绿色表示可建造，红色表示不可建造
 */
UCLASS()
class VALORIS_API ABuildPreview : public AActor
{
	GENERATED_BODY()

public:
	ABuildPreview();

	// 设置预览的塔类型
	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetTowerClass(TSubclassOf<ATowerBase> InTowerClass);

	// 更新预览位置
	UFUNCTION(BlueprintCallable, Category = "Build")
	void UpdatePreviewLocation(const FVector& Location);

	// 检查当前位置是否可建造
	UFUNCTION(BlueprintCallable, Category = "Build")
	bool CanBuildAtCurrentLocation() const;

	// 设置预览可见性
	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetPreviewVisible(bool bVisible);

protected:
	// 更新预览材质（绿色/红色）
	void UpdatePreviewMaterial();

	// 检查是否与其他物体重叠
	bool CheckOverlap() const;

	//~ 组件

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

	//~ 配置

	// 可建造时的材质
	UPROPERTY(EditDefaultsOnly, Category = "Build|Materials")
	TObjectPtr<UMaterialInterface> ValidMaterial;

	// 不可建造时的材质
	UPROPERTY(EditDefaultsOnly, Category = "Build|Materials")
	TObjectPtr<UMaterialInterface> InvalidMaterial;

	// 检测重叠的半径
	UPROPERTY(EditDefaultsOnly, Category = "Build|Config")
	float OverlapCheckRadius = 100.f;

	//~ 状态

	// 当前预览的塔类型
	UPROPERTY()
	TSubclassOf<ATowerBase> CurrentTowerClass;

	// 当前位置是否可建造
	bool bCanBuild = false;
};
