// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowSubsystem.h"

#include "GlowImage.h"
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

void UGlowSubsystem::UpdateGlowSource(UWidget* Widget, UMaterialInstanceDynamic* DMI) const
{
    const FGeometry& Geometry = Widget->GetCachedGeometry();
    
    
    const FVector2D AbsoluteCorners[4] = {
        Geometry.GetAbsolutePosition(),
        Geometry.GetAbsolutePositionAtCoordinates(FVector2D(0,1)),
        Geometry.GetAbsolutePositionAtCoordinates(FVector2D(1,0)),
        Geometry.GetAbsolutePositionAtCoordinates(FVector2D(1,1))
    };
    
    if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
    {
        const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(Widget);
        const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
        FVector2D MaterialUV[4];
        for (int32 i = 0; i < 4; ++i)
        {
            const FVector2D AbsPoint = AbsoluteCorners[i];

            FVector2D PixelPoint, ViewportPoint;
            USlateBlueprintLibrary::AbsoluteToViewport(World, AbsPoint, PixelPoint, ViewportPoint);
            MaterialUV[i] = (ViewportSize.X > 0.f && ViewportSize.Y > 0.f)
                ? ViewportScale * (ViewportPoint / ViewportSize)
                : FVector2D::ZeroVector;
        }
        
        DMI->SetVectorParameterValue(TEXT("QuadCorner0"), FLinearColor(MaterialUV[0].X, MaterialUV[0].Y, 0, 0));
        DMI->SetVectorParameterValue(TEXT("QuadCorner1"), FLinearColor(MaterialUV[1].X, MaterialUV[1].Y, 0, 0));
        DMI->SetVectorParameterValue(TEXT("QuadCorner2"), FLinearColor(MaterialUV[2].X, MaterialUV[2].Y, 0, 0));
        DMI->SetVectorParameterValue(TEXT("QuadCorner3"), FLinearColor(MaterialUV[3].X, MaterialUV[3].Y, 0, 0));
    }
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