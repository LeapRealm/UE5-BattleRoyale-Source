#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorldMapWidget.generated.h"

class UElectricCircleWidget;
class UBorder;
class UImage;
class UCanvasPanel;
class UMiniMapWidget;
class UCanvasPanelSlot;
class UCompassBarWidget;

UCLASS()
class BATTLEROYAL_API UWorldMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION()
	bool PinIconOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	
private:
	float GetCurrentWorldMapZoom();
	FVector InitialWidgetPosToWorldPos();
	FVector2D WorldPosToInitialWidgetPos(const FVector& WorldPos);
	float WorldLengthToInitialWidgetLength(float WorldLength);

public:
	void SetMiniMapWidget(UMiniMapWidget* Widget) { MiniMapWidget = Widget; }
	void SetCompassBarWidget(UCompassBarWidget* Widget) { CompassBarWidget = Widget; }
	void ShowAircraftUI();
	void HideAircraftUI();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> PinIconSlot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> PlayerPanelSlot;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> WorldMapPanelSlot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> AircraftLineSlot;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> AircraftIconSlot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> CurrCircleSlot;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCanvasPanelSlot> TargetCircleSlot;
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMiniMapWidget> MiniMapWidget;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCompassBarWidget> CompassBarWidget;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> PinIcon;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> PlayerPanel;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> WorldMapPanel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> AircraftLine;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> AircraftIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UElectricCircleWidget> CurrCircleWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UElectricCircleWidget> TargetCircleWidget;

private:
	float MinWorldMapZoom = 1.f;
	float MaxWorldMapZoom = 16.f;
	float DeltaWorldMapZoom = 2.f;

	float InitialInterpSpeed = 15.f;
	FVector2D InitialWorldMapSize;
	FVector2D InitialPinIconPos;

	float TargetWorldMapZoom = 1.f;
	FVector2D TargetWorldMapSize;
	FVector2D TargetWorldMapPos;

	float SizeConstant;

private:
	FVector2D WorldFirstPos = FVector2D(79270.f, -149770.f);
	FVector2D WorldSecondPos = FVector2D(272410.f, -262810.f);
	
	FVector2D WidgetFirstPos = FVector2D(140.8f, 328.5f);
	FVector2D WidgetSecondPos = FVector2D(-164.f, -190.f);
};
