// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Components/PostProcessComponent.h"
#include "GlowSubsystem.generated.h"

class UWidget;
class UMaterialInstanceDynamic;

UCLASS()
class GLOWUI_API UGlowSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// FTickableGameObject — GameInstanceSubsystem doesn't tick on its own,
	// this is how it gets a per-frame hook.
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }

	void RegisterGlowWidget(UWidget* Widget, UMaterialInstanceDynamic* CachedDMI);
	void UnregisterGlowWidget(UWidget* Widget);

private:
	// Pivot-relative scaled corner — matches the formula validated
	// against the Week 2 Blueprint log data.
	static FVector2D ScaleCornerAroundPivot(
		const FVector2D& LocalCorner,
		const FVector2D& PivotLocal,
		const FVector2D& Scale);

	void UpdateGlowSource(UWidget* Widget, UMaterialInstanceDynamic* DMI);

	UPROPERTY()
	TObjectPtr<AActor> GlowRigActor;
	
	UPROPERTY()
	TObjectPtr<UPostProcessComponent> PostProcessComponent;
	
	void EnsurePostProcessComponent();
	
	UPROPERTY()
	TMap<TWeakObjectPtr<UWidget>, TWeakObjectPtr<UMaterialInstanceDynamic>> RegisteredGlowSources;
};