// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "ValorisSpectatorPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

/**
 * RTS风格相机Pawn
 * 支持边缘平移、滚轮缩放、中键旋转、空格聚焦英雄
 */
UCLASS()
class VALORIS_API AValorisSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AValorisSpectatorPawn();

	virtual void Tick(float DeltaTime) override;

	// 相机移动
	void PanCamera(const FVector2D& Direction);

	// 相机缩放
	void ZoomCamera(float Amount);

	// 相机旋转
	void RotateCamera(float Amount);

	// 聚焦到指定位置
	void FocusOnLocation(const FVector& Location);

protected:
	virtual void BeginPlay() override;

	// 弹簧臂组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	// 相机组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// 相机移动速度
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Movement")
	float PanSpeed = 1500.f;

	// 相机缩放速度
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom")
	float ZoomSpeed = 200.f;

	// 最小缩放距离
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom")
	float MinZoomDistance = 500.f;

	// 最大缩放距离
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom")
	float MaxZoomDistance = 5000.f;

	// 相机旋转速度
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Rotation")
	float RotateSpeed = 100.f;

	// 相机俯仰角
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Rotation")
	float CameraPitch = -55.f;

	// 目标缩放距离（用于平滑插值）
	float TargetArmLength;

private:
	// 处理屏幕边缘平移
	void HandleEdgePanning(float DeltaTime);

	// 边缘检测阈值（像素）
	UPROPERTY(EditDefaultsOnly, Category = "Camera|EdgePan")
	float EdgePanThreshold = 20.f;

	// 是否启用边缘平移
	UPROPERTY(EditDefaultsOnly, Category = "Camera|EdgePan")
	bool bEnableEdgePanning = true;
};
