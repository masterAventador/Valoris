// Copyright Valoris. All Rights Reserved.

#include "ValorisPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "../Camera/ValorisSpectatorPawn.h"
#include "../Character/ValorisCharacterBase.h"
#include "../Character/HeroAIController.h"
#include "../Character/AricHero.h"
#include "../Enemy/EnemyBase.h"

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

	// 限制鼠标在视口内（仅打包后生效）
#if !WITH_EDITOR
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
#endif

	// 查找场景中的英雄（用 AAricHero 类型，避免找到敌人）
	AActor* FoundHero = UGameplayStatics::GetActorOfClass(GetWorld(), AAricHero::StaticClass());
	if (FoundHero)
	{
		ControlledHero = Cast<AValorisCharacterBase>(FoundHero);
	}
}

void AValorisPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 右键点击
		if (RightClickAction)
		{
			EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnRightClick);
		}

		// 相机缩放
		if (CameraZoomAction)
		{
			EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AValorisPlayerController::OnCameraZoom);
		}

		// 相机旋转
		if (CameraRotateAction)
		{
			EnhancedInputComponent->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &AValorisPlayerController::OnCameraRotate);
		}

		// 聚焦英雄
		if (FocusHeroAction)
		{
			EnhancedInputComponent->BindAction(FocusHeroAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnFocusHero);
		}
	}
}

AValorisSpectatorPawn* AValorisPlayerController::GetCameraPawn() const
{
	return Cast<AValorisSpectatorPawn>(GetPawn());
}

void AValorisPlayerController::SetControlledHero(AValorisCharacterBase* Hero)
{
	ControlledHero = Hero;
}

AValorisCharacterBase* AValorisPlayerController::GetControlledHero() const
{
	return ControlledHero;
}

void AValorisPlayerController::OnRightClick(const FInputActionValue& Value)
{
	if (!ControlledHero)
	{
		return;
	}

	AHeroAIController* HeroAI = Cast<AHeroAIController>(ControlledHero->GetController());
	if (!HeroAI)
	{
		return;
	}

	// 检测是否点击了敌人
	AActor* ClickedActor = GetActorUnderCursor();
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(ClickedActor))
	{
		// 点击敌人，设置攻击目标
		HeroAI->SetAttackTarget(Enemy);
	}
	else
	{
		// 点击地面，移动
		FVector HitLocation;
		if (GetMouseHitLocation(HitLocation))
		{
			HeroAI->ClearAttackTarget();
			HeroAI->MoveToTargetLocation(HitLocation);
		}
	}
}

void AValorisPlayerController::OnCameraZoom(const FInputActionValue& Value)
{
	if (AValorisSpectatorPawn* CameraPawn = GetCameraPawn())
	{
		const float ZoomValue = Value.Get<float>();
		CameraPawn->ZoomCamera(ZoomValue);
	}
}

void AValorisPlayerController::OnCameraRotate(const FInputActionValue& Value)
{
	if (AValorisSpectatorPawn* CameraPawn = GetCameraPawn())
	{
		const float RotateValue = Value.Get<float>();
		CameraPawn->RotateCamera(RotateValue);
	}
}

void AValorisPlayerController::OnFocusHero(const FInputActionValue& Value)
{
	if (AValorisSpectatorPawn* CameraPawn = GetCameraPawn())
	{
		if (ControlledHero)
		{
			CameraPawn->FocusOnLocation(ControlledHero->GetActorLocation());
		}
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

AActor* AValorisPlayerController::GetActorUnderCursor() const
{
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Pawn, false, HitResult))
	{
		return HitResult.GetActor();
	}
	return nullptr;
}
