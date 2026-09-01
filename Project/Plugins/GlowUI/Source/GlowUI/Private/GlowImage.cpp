// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowImage.h"
#include "GlowSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY(LogGlowImage)

TSharedRef<SWidget> UGlowImage::RebuildWidget()
{
	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s RebuildWidget called"), *GetName());
	InitializeGlow();
	return Widget;
}

void UGlowImage::ReleaseSlateResources(bool bReleaseChildren)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGlowSubsystem* Subsystem = GameInstance->GetSubsystem<UGlowSubsystem>())
			{
				Subsystem->UnregisterGlowWidget(this);
				UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s unregistered from subsystem"), *GetName());
			}
		}
	}

	GlowDMI = nullptr;
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UGlowImage::OnInitialized()
{
	Super::OnInitialized();
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s OnInitialized called"), *GetName());
	InitializeGlow();
}

void UGlowImage::InitializeGlow()
{
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s InitializeGlow called — bGlowEnabled=%d, GlowMaterialParent=%s, GlowDMI=%s"),
		*GetName(), bGlowEnabled, GlowMaterialParent ? TEXT("valid") : TEXT("null"), GlowDMI ? TEXT("exists") : TEXT("null"));

	if (!bGlowEnabled)
	{
		UE_LOG(LogGlowImage, Warning, TEXT("GlowImage %s: bGlowEnabled is false — skipping initialization"), *GetName());
		return;
	}
	if (!GlowMaterialParent)
	{
		UE_LOG(LogGlowImage, Warning, TEXT("GlowImage %s: GlowMaterialParent is not set — assign the PP material in Blueprint"), *GetName());
		return;
	}
	if (GlowDMI)
	{
		UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s: GlowDMI already exists — skipping recreation"), *GetName());
		return;
	}

	GlowDMI = UMaterialInstanceDynamic::Create(GlowMaterialParent, this);
	if (!GlowDMI)
	{
		UE_LOG(LogGlowImage, Error, TEXT("Failed to create MaterialInstanceDynamic for GlowImage %s"), *GetName());
		return;
	}

	GlowDMI->SetVectorParameterValue(TEXT("GlowColor"), GlowColor);
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s: GlowColor set to %s"), *GetName(), *GlowColor.ToString());
	GlowDMI->SetScalarParameterValue(TEXT("Intensity"), GlowIntensity);
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s: GlowIntensity set to %f"), *GetName(), GlowIntensity);

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGlowSubsystem* Subsystem = GameInstance->GetSubsystem<UGlowSubsystem>())
			{
				Subsystem->RegisterGlowWidget(this, GlowDMI);
				UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s: Registered with subsystem"), *GetName());
			}
			else
			{
				UE_LOG(LogGlowImage, Error, TEXT("GlowImage %s: GlowSubsystem not found — module not loaded?"), *GetName());
			}
		}
		else
		{
			UE_LOG(LogGlowImage, Error, TEXT("GlowImage %s: GetGameInstance returned null"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogGlowImage, Error, TEXT("GlowImage %s: GetWorld returned null — widget not in a world"), *GetName());
	}
}
