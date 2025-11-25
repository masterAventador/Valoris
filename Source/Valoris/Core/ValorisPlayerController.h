// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ValorisPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 玩家控制器
 * 处理输入、相机控制、点击移动等
 */
UCLASS()
class VALORIS_API AValorisPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AValorisPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 输入映射上下文
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 点击移动
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RightClickAction;

	// 处理右键点击
	void OnRightClick(const FInputActionValue& Value);

private:
	// 获取鼠标点击位置的世界坐标
	bool GetMouseHitLocation(FVector& OutLocation) const;
};
