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
	RegisterWithSubsystem();
	return Widget;}

void UGlowImage::ReleaseSlateResources(bool bReleaseChildren)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGlowSubsystem* Subsystem = GameInstance->GetSubsystem<UGlowSubsystem>())
			{
				Subsystem->UnregisterGlowWidget(this);
			}
		}
	}

	GlowDMI = nullptr;
	Super::ReleaseSlateResources(bReleaseChildren);}

void UGlowImage::RegisterWithSubsystem()
{
	if (!bGlowEnabled || !GlowMaterialParent || GlowDMI)
	{
		return; // disabled, unconfigured, or already registered — don't recreate
	}

	GlowDMI = UMaterialInstanceDynamic::Create(GlowMaterialParent, this);
	if (!GlowDMI)
	{
		UE_LOG(LogGlowImage, Error, TEXT("Failed to create MaterialInstanceDynamic for GlowImage %s"), *GetName());
		return;
	}

	GlowDMI->SetVectorParameterValue(TEXT("GlowColor"), GlowColor);
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s has GlowColor %s"), *GetName(), *GlowColor.ToString());
	GlowDMI->SetScalarParameterValue(TEXT("Intensity"), GlowIntensity);
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s has GlowIntensity %f"), *GetName(), GlowIntensity);

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGlowSubsystem* Subsystem = GameInstance->GetSubsystem<UGlowSubsystem>())
			{
				Subsystem->RegisterGlowWidget(this, GlowDMI);
			}
		}
	}
}
