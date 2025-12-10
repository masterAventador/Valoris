// Copyright Valoris. All Rights Reserved.

#include "ValorisPlayerController.h"
#include "ValorisGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "../Camera/ValorisSpectatorPawn.h"
#include "../Character/ValorisCharacterBase.h"
#include "../Character/HeroAIController.h"
#include "../Character/AricHero.h"
#include "../Enemy/EnemyBase.h"
#include "../Tower/TowerBase.h"
#include "../Building/BuildPreview.h"
#include "../Economy/ResourceManager.h"
#include "../UI/ValorisHUD.h"

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

	// 创建 HUD
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UValorisHUD>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
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

		// 左键点击
		if (LeftClickAction)
		{
			EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnLeftClick);
		}

		// 取消键
		if (CancelAction)
		{
			EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnCancel);
		}

		// 建造模式键
		if (BuildModeAction)
		{
			EnhancedInputComponent->BindAction(BuildModeAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnBuildMode);
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

void AValorisPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 建造模式下更新预览位置
	if (bInBuildMode)
	{
		UpdateBuildPreview();
	}
}

void AValorisPlayerController::OnLeftClick(const FInputActionValue& Value)
{
	if (bInBuildMode)
	{
		ConfirmBuild();
	}
}

void AValorisPlayerController::OnCancel(const FInputActionValue& Value)
{
	if (bInBuildMode)
	{
		ExitBuildMode();
	}
}

void AValorisPlayerController::OnBuildMode(const FInputActionValue& Value)
{
	if (bInBuildMode)
	{
		// 已在建造模式，退出
		ExitBuildMode();
	}
	else
	{
		// 进入建造模式
		if (DefaultTowerClass)
		{
			EnterBuildMode(DefaultTowerClass);
		}
	}
}

void AValorisPlayerController::EnterBuildMode(TSubclassOf<ATowerBase> TowerClass)
{
	if (!TowerClass)
	{
		return;
	}

	CurrentTowerClass = TowerClass;
	bInBuildMode = true;

	// 创建建造预览
	if (BuildPreviewClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		BuildPreviewActor = GetWorld()->SpawnActor<ABuildPreview>(BuildPreviewClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (BuildPreviewActor)
		{
			BuildPreviewActor->SetTowerClass(TowerClass);
			BuildPreviewActor->SetPreviewVisible(true);
		}
	}
}

void AValorisPlayerController::ExitBuildMode()
{
	bInBuildMode = false;
	CurrentTowerClass = nullptr;

	// 销毁建造预览
	if (BuildPreviewActor)
	{
		BuildPreviewActor->Destroy();
		BuildPreviewActor = nullptr;
	}
}

void AValorisPlayerController::ConfirmBuild()
{
	if (!bInBuildMode || !CurrentTowerClass || !BuildPreviewActor)
	{
		return;
	}

	// 检查是否可以建造
	if (!BuildPreviewActor->CanBuildAtCurrentLocation())
	{
		return;
	}

	// 获取塔的建造花费
	const ATowerBase* TowerCDO = CurrentTowerClass->GetDefaultObject<ATowerBase>();
	const int32 BuildCost = TowerCDO ? TowerCDO->GetBuildCost() : 0;

	// 获取资源管理器
	UResourceManager* ResourceManager = nullptr;
	if (AValorisGameMode* GameMode = Cast<AValorisGameMode>(GetWorld()->GetAuthGameMode()))
	{
		ResourceManager = GameMode->GetResourceManager();
	}

	// 检查资源是否足够
	if (!ResourceManager || !ResourceManager->HasEnoughGold(BuildCost))
	{
		return;
	}

	// 生成塔
	FVector TowerLocation = BuildPreviewActor->GetActorLocation();
	FRotator TowerRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATowerBase* NewTower = GetWorld()->SpawnActor<ATowerBase>(CurrentTowerClass, TowerLocation, TowerRotation, SpawnParams);
	if (NewTower)
	{
		// 扣除资源
		ResourceManager->SpendGold(BuildCost);

		// 退出建造模式
		ExitBuildMode();
	}
}

void AValorisPlayerController::UpdateBuildPreview()
{
	if (!BuildPreviewActor)
	{
		return;
	}

	FVector HitLocation;
	if (GetMouseHitLocation(HitLocation))
	{
		BuildPreviewActor->UpdatePreviewLocation(HitLocation);
	}
}
