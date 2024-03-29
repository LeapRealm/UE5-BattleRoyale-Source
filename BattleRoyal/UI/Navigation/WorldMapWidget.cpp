#include "WorldMapWidget.h"

#include "BattleRoyalGameState.h"
#include "BattleRoyalPlayerController.h"
#include "MiniMapWidget.h"
#include "CompassBarWidget.h"
#include "ElectricCircleWidget.h"
#include "Aircraft.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void UWorldMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	bIsFocusable = true;
	
	PinIconSlot = Cast<UCanvasPanelSlot>(PinIcon->Slot);
	PlayerPanelSlot = Cast<UCanvasPanelSlot>(PlayerPanel->Slot);
	WorldMapPanelSlot = Cast<UCanvasPanelSlot>(WorldMapPanel->Slot);
	AircraftLineSlot = Cast<UCanvasPanelSlot>(AircraftLine->Slot);
	AircraftIconSlot = Cast<UCanvasPanelSlot>(AircraftIcon->Slot);
	CurrCircleSlot = Cast<UCanvasPanelSlot>(CurrCircleWidget->Slot);
	TargetCircleSlot = Cast<UCanvasPanelSlot>(TargetCircleWidget->Slot);
	
	InitialWorldMapSize = WorldMapPanelSlot->GetSize();
	TargetWorldMapSize = InitialWorldMapSize;

	PinIcon->SetVisibility(ESlateVisibility::Hidden);
	PinIcon->OnMouseButtonDownEvent.BindUFunction(this, TEXT("PinIconOnMouseButtonDown"));

	AircraftLine->SetVisibility(ESlateVisibility::Hidden);
	AircraftIcon->SetVisibility(ESlateVisibility::Hidden);

	SizeConstant = (WorldPosToInitialWidgetPos(FVector(100.f, 0.f, 0.f)) - WorldPosToInitialWidgetPos(FVector(0.f, 0.f, 0.f))).Length();
}

void UWorldMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Refresh World Map Panel Size & Position
	FVector2D NewSize = FMath::Vector2DInterpTo(WorldMapPanelSlot->GetSize(), TargetWorldMapSize, InDeltaTime, InitialInterpSpeed);
	WorldMapPanelSlot->SetSize(NewSize);

	FVector2D NewPos = FMath::Vector2DInterpTo(WorldMapPanelSlot->GetPosition(), TargetWorldMapPos, InDeltaTime, InitialInterpSpeed);
	WorldMapPanelSlot->SetPosition(NewPos);
	
	if (FMath::IsNearlyEqual(TargetWorldMapZoom, MinWorldMapZoom))
	{
		// Reset World Map Panel Target Position
		TargetWorldMapPos = FVector2D::ZeroVector;
	}
	else
	{
		// Clamp World Map Panel Position
		FVector2D DeltaSize = WorldMapPanelSlot->GetSize() / 2.f;
		FVector2D WorldMapPos = WorldMapPanelSlot->GetPosition();
		float x = FMath::Clamp(WorldMapPos.X, -DeltaSize.X, DeltaSize.X);
		float y = FMath::Clamp(WorldMapPos.Y, -DeltaSize.Y, DeltaSize.Y);
		WorldMapPanelSlot->SetPosition(FVector2D(x, y));
	}

	// Refresh Player Panel Position/Rotation & Pin Icon Position
	APawn* Pawn = GetOwningPlayerPawn();
	if (IsValid(Pawn))
	{
		FVector2D InitialPos = WorldPosToInitialWidgetPos(Pawn->GetActorLocation());
		PlayerPanelSlot->SetPosition(InitialPos * GetCurrentWorldMapZoom());
	}

	if (ABattleRoyalPlayerController* ControllerOwner = Cast<ABattleRoyalPlayerController>(GetOwningPlayer()))
	{
		if (AircraftIcon->GetVisibility() == ESlateVisibility::Visible)
		{
			AAircraft* Aircraft = ControllerOwner->GetAircraft();
			if (IsValid(Aircraft))
			{
				FVector2D InitialPos = WorldPosToInitialWidgetPos(Aircraft->GetActorLocation());
				AircraftIconSlot->SetPosition(InitialPos * GetCurrentWorldMapZoom());
				float Angle = Aircraft->GetActorRotation().Yaw;
				AircraftIcon->SetRenderTransformAngle(Angle);
				AircraftIcon->SetRenderScale(FVector2D(GetCurrentWorldMapZoom()));

				InitialPos = WorldPosToInitialWidgetPos((Aircraft->StartLocation + Aircraft->AllDiveLocation) / 2.f);
				AircraftLineSlot->SetPosition(InitialPos * GetCurrentWorldMapZoom());
				AircraftLine->SetRenderTransformAngle(-Angle);
				AircraftLine->SetRenderScale(FVector2D(GetCurrentWorldMapZoom()));
			}
			else
			{
				HideAircraftUI();
			}
		}
		
		if (APlayerCameraManager* CameraManager = ControllerOwner->PlayerCameraManager)
			PlayerPanel->SetRenderTransformAngle(CameraManager->GetCameraRotation().Yaw);
	}

	PinIconSlot->SetPosition(InitialPinIconPos * GetCurrentWorldMapZoom());

	// Refresh Circle Widget
	if (ABattleRoyalGameState* GameState = Cast<ABattleRoyalGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (GameState->bVisibleCurrCircle)
			CurrCircleWidget->SetVisibility(ESlateVisibility::Visible);
		else
			CurrCircleWidget->SetVisibility(ESlateVisibility::Hidden);

		if (GameState->bVisibleTargetCircle)
			TargetCircleWidget->SetVisibility(ESlateVisibility::Visible);
		else
			TargetCircleWidget->SetVisibility(ESlateVisibility::Hidden);
		
		FVector2D InitialPos = WorldPosToInitialWidgetPos(GameState->CurrCirclePos);
		CurrCircleSlot->SetPosition(InitialPos * GetCurrentWorldMapZoom());

		FVector2D InitialSize = FVector2D(WorldLengthToInitialWidgetLength(GameState->CurrCircleRadius * 2));
		CurrCircleSlot->SetSize(InitialSize * GetCurrentWorldMapZoom());
		
		InitialPos = WorldPosToInitialWidgetPos(GameState->TargetCirclePos);
		TargetCircleSlot->SetPosition(InitialPos * GetCurrentWorldMapZoom());

		InitialSize = InitialSize = FVector2D(WorldLengthToInitialWidgetLength(GameState->TargetCircleRadius * 2));
		TargetCircleSlot->SetSize(InitialSize * GetCurrentWorldMapZoom());
	}
}

FReply UWorldMapWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	
	if (InKeyEvent.IsRepeat() == false && InKeyEvent.GetKey() == EKeys::M && GetVisibility() == ESlateVisibility::Visible)
	{
		SetVisibility(ESlateVisibility::Hidden);
		
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController, true);
			PlayerController->SetShowMouseCursor(false);
		}
		
		return FReply::Handled();
	}
    
	return Reply;
}

FReply UWorldMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && TargetWorldMapZoom > MinWorldMapZoom)
	{
		TargetWorldMapPos = WorldMapPanelSlot->GetPosition() + InMouseEvent.GetCursorDelta();
		WorldMapPanelSlot->SetPosition(TargetWorldMapPos);
		return FReply::Handled();
	}
	
	return Reply;
}

FReply UWorldMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseWheel(InGeometry, InMouseEvent);

	if (FMath::Abs(TargetWorldMapZoom - GetCurrentWorldMapZoom()) > 0.25f)
		return Reply;
	
	if (InMouseEvent.GetWheelDelta() > 0.f)
	{
		if (TargetWorldMapZoom < MaxWorldMapZoom)
		{
			FVector2D WorldMapPanelHalfSize = WorldMapPanelSlot->GetSize() / 2.f;
			FVector2D WorldMapPanelPos = WorldMapPanelSlot->GetPosition();
			FVector2D MouseLocalPos = WorldMapPanel->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
			TargetWorldMapPos = -WorldMapPanelHalfSize + WorldMapPanelPos - MouseLocalPos + (WorldMapPanelHalfSize * DeltaWorldMapZoom);
		}

		TargetWorldMapZoom = FMath::Clamp(TargetWorldMapZoom * DeltaWorldMapZoom, MinWorldMapZoom, MaxWorldMapZoom);
		TargetWorldMapSize = InitialWorldMapSize * TargetWorldMapZoom;
	}
	else
	{
		if (TargetWorldMapZoom > MinWorldMapZoom * DeltaWorldMapZoom)
		{
			FVector2D WorldMapPanelHalfSize = WorldMapPanelSlot->GetSize() / 2.f;
			FVector2D WorldMapPanelPos = WorldMapPanelSlot->GetPosition();
			FVector2D MouseLocalPos = WorldMapPanel->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
			TargetWorldMapPos = -WorldMapPanelHalfSize + WorldMapPanelPos + (MouseLocalPos / DeltaWorldMapZoom) + (WorldMapPanelHalfSize / DeltaWorldMapZoom);
		}

		TargetWorldMapZoom = FMath::Clamp(TargetWorldMapZoom / DeltaWorldMapZoom, MinWorldMapZoom, MaxWorldMapZoom);
		TargetWorldMapSize = InitialWorldMapSize * TargetWorldMapZoom;
	}
	
	return FReply::Handled();
}

FReply UWorldMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (WorldMapPanel->IsHovered() && InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		FVector2D MouseLocalPos = WorldMapPanel->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		FVector2D WorldMapPanelHalfSize = WorldMapPanelSlot->GetSize() / 2.f;
		FVector2D PinIconSlotPos = MouseLocalPos - WorldMapPanelHalfSize;
		InitialPinIconPos = PinIconSlotPos / GetCurrentWorldMapZoom();
		
		PinIconSlot->SetPosition(PinIconSlotPos);
		PinIcon->SetVisibility(ESlateVisibility::Visible);

		FVector WorldPos = InitialWidgetPosToWorldPos();
		MiniMapWidget->ShowPinIcon(WorldPos);
		CompassBarWidget->ShowPinIcon(WorldPos);
		
		return FReply::Handled();
	}

	return Reply;
}

bool UWorldMapWidget::PinIconOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		PinIcon->SetVisibility(ESlateVisibility::Hidden);
		MiniMapWidget->HidePinIcon();
		CompassBarWidget->HidePinIcon();
		return true;
	}
	return false;
}

float UWorldMapWidget::GetCurrentWorldMapZoom()
{
	return (WorldMapPanelSlot->GetSize() / InitialWorldMapSize).X;
}

FVector UWorldMapWidget::InitialWidgetPosToWorldPos()
{
	float x = FMath::GetMappedRangeValueUnclamped(
		FVector2D(WidgetFirstPos.Y, WidgetSecondPos.Y),
		FVector2D(WorldFirstPos.X, WorldSecondPos.X),
		InitialPinIconPos.Y);
	
	float y = FMath::GetMappedRangeValueUnclamped(
		FVector2D(WidgetFirstPos.X, WidgetSecondPos.X),
		FVector2D(WorldFirstPos.Y, WorldSecondPos.Y),
		InitialPinIconPos.X);

	return FVector(x, y, 0.f);
}

FVector2D UWorldMapWidget::WorldPosToInitialWidgetPos(const FVector& WorldPos)
{
	float x = FMath::GetMappedRangeValueUnclamped(
		FVector2D(WorldFirstPos.Y, WorldSecondPos.Y),
		FVector2D(WidgetFirstPos.X, WidgetSecondPos.X),
		WorldPos.Y);

	float y = FMath::GetMappedRangeValueUnclamped(
		FVector2D(WorldFirstPos.X, WorldSecondPos.X),
		FVector2D(WidgetFirstPos.Y, WidgetSecondPos.Y),
		WorldPos.X);
	
	return FVector2D(x, y);
}

float UWorldMapWidget::WorldLengthToInitialWidgetLength(float WorldLength)
{
	return WorldLength * (SizeConstant / 100.f);
}

void UWorldMapWidget::ShowAircraftUI()
{
	AircraftLine->SetVisibility(ESlateVisibility::Visible);
	AircraftIcon->SetVisibility(ESlateVisibility::Visible);
}

void UWorldMapWidget::HideAircraftUI()
{
	AircraftLine->SetVisibility(ESlateVisibility::Hidden);
	AircraftIcon->SetVisibility(ESlateVisibility::Hidden);
}
