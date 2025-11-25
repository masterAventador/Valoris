// Copyright Valoris. All Rights Reserved.

#include "ValorisPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AValorisPlayerController::AValorisPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AValorisPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 添加输入映射上下文
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AValorisPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (RightClickAction)
		{
			EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnRightClick);
		}
	}
}

void AValorisPlayerController::OnRightClick(const FInputActionValue& Value)
{
	FVector HitLocation;
	if (GetMouseHitLocation(HitLocation))
	{
		// TODO: 通知英雄移动到目标位置
		UE_LOG(LogTemp, Log, TEXT("Right click at: %s"), *HitLocation.ToString());
	}
}

bool AValorisPlayerController::GetMouseHitLocation(FVector& OutLocation) const
{
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		OutLocation = HitResult.Location;
		return true;
	}
	return false;
}
