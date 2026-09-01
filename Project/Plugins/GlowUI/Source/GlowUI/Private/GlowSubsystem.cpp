// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowSubsystem.h"
#include "Components/Widget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UGlowSubsystem::Tick(float DeltaTime)
{
    for (auto It = RegisteredGlowSources.CreateIterator(); It; ++It)
    {
        UWidget* Widget = It->Key.Get();
        UMaterialInstanceDynamic* DMI = It->Value.Get();

        if (!Widget || !DMI)
        {
            It.RemoveCurrent();
            continue;
        }

        UpdateGlowSource(Widget, DMI);
    }
}

TStatId UGlowSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UGlowSubsystem, STATGROUP_Tickables);
}

void UGlowSubsystem::RegisterGlowWidget(UWidget* Widget, UMaterialInstanceDynamic* CachedDMI)
{
    if (Widget && CachedDMI)
    {
        RegisteredGlowSources.Add(Widget, CachedDMI);
    }
    else
    {
        return;
    }

    EnsurePostProcessComponent();

    if (PostProcessComponent)
    {
        PostProcessComponent->AddOrUpdateBlendable(CachedDMI, 1.0f);
    }
}

void UGlowSubsystem::UnregisterGlowWidget(UWidget* Widget)
{
    RegisteredGlowSources.Remove(Widget);
}

FVector2D UGlowSubsystem::ScaleCornerAroundPivot(
    const FVector2D& LocalCorner,
    const FVector2D& PivotLocal,
    const FVector2D& Scale)
{
    return PivotLocal + (LocalCorner - PivotLocal) * Scale;
}

void UGlowSubsystem::UpdateGlowSource(UWidget* Widget, UMaterialInstanceDynamic* DMI)
{
    const FGeometry& Geometry = Widget->GetCachedGeometry();

    const FVector2D LocalTopLeft = Geometry.GetLocalTopLeft();
    const FVector2D LocalSize = Geometry.GetLocalSize();
    const FVector2D LocalBottomRight = LocalTopLeft + LocalSize;

    const FVector2D PivotNormalized(0.5f, 0.5f);
    const FVector2D PivotLocal = LocalTopLeft + PivotNormalized * LocalSize;
    const FVector2D Scale(1.0f, 1.0f);

    const FVector2D ScaledTopLeft = ScaleCornerAroundPivot(LocalTopLeft, PivotLocal, Scale);
    const FVector2D ScaledBottomRight = ScaleCornerAroundPivot(LocalBottomRight, PivotLocal, Scale);

    const FVector2D AbsMin = Geometry.LocalToAbsolute(ScaledTopLeft);
    const FVector2D AbsMax = Geometry.LocalToAbsolute(ScaledBottomRight);

    FVector2D ViewportMin = AbsMin;
    FVector2D ViewportMax = AbsMax;

    if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
    {
        ViewportMin = USlateBlueprintLibrary::AbsoluteToViewport(World, AbsMin);
        ViewportMax = USlateBlueprintLibrary::AbsoluteToViewport(World, AbsMax);
    }

    const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
    const FVector2D RectMin = ViewportSize.X > 0.f ? ViewportMin / ViewportSize : FVector2D::ZeroVector;
    const FVector2D RectMax = ViewportSize.Y > 0.f ? ViewportMax / ViewportSize : FVector2D::ZeroVector;

    DMI->SetVectorParameterValue(TEXT("RectMin"), FLinearColor(RectMin.X, RectMin.Y, 0, 0));
    DMI->SetVectorParameterValue(TEXT("RectMax"), FLinearColor(RectMax.X, RectMax.Y, 0, 0));
}

void UGlowSubsystem::EnsurePostProcessComponent()
{
    if (PostProcessComponent)
    {
        return;
    }

    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (!World)
    {
        return;
    }

    GlowRigActor = World->SpawnActor<AActor>();
    if (!GlowRigActor)
    {
        return;
    }

    PostProcessComponent = NewObject<UPostProcessComponent>(GlowRigActor);
    PostProcessComponent->bUnbound = true;
    PostProcessComponent->RegisterComponent();
    GlowRigActor->AddInstanceComponent(PostProcessComponent);
}