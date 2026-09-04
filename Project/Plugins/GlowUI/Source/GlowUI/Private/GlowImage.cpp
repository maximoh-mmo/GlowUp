// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowImage.h"
#include "GlowSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Slate/SlateTextureAtlasInterface.h"

DEFINE_LOG_CATEGORY(LogGlowImage)
TSharedRef<SWidget> UGlowImage::RebuildWidget()
{
	EnsureBrushConstraints();
	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s has been rebuilt"), *GetName());
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

void UGlowImage::InitializeGlow()
{
	UE_LOG(LogGlowImage, Log, TEXT("GlowImage %s has been initialized"), *GetName());
	RegisterWithSubsystem();
}

void UGlowImage::RegisterWithSubsystem()
{
	if (!bGlowEnabled || !GlowMaterialParent)
	{
		return;
	}

	if (!GlowDMI)
	{
		GlowDMI = UMaterialInstanceDynamic::Create(GlowMaterialParent, this);
		if (!GlowDMI)
		{
			return;
		}
		GlowDMI->SetVectorParameterValue(TEXT("GlowColor"), GlowColor);
		GlowDMI->SetScalarParameterValue(TEXT("Intensity"), GlowIntensity);
	}

	// Always (re-)register, even if GlowDMI already existed — ReleaseSlateResources
	// may have unregistered this widget from the subsystem during a sibling-triggered
	// rebuild without GlowDMI itself being destroyed. Registering with an already-known
	// DMI should just update its entry, not duplicate it.
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

void UGlowImage::EnsureBrushConstraints()
{
	FSlateBrush Fixed = GetBrush();
	bool bChanged = false;

	// Draw As is forced to Image: Box/Border/RoundedBox margins break the
	// 1:1 mapping between widget bounds and glow quad. Existing content using
	// other modes is reverted automatically (with a warning).
	if (Fixed.GetDrawType() != ESlateBrushDrawType::Image)
	{
		UE_LOG(LogGlowImage, Warning, TEXT("GlowImage %s: Draw As mode %d is unsupported for glow tracking — reverted to Image."),
			*GetName(), static_cast<int32>(Fixed.GetDrawType()));
		Fixed.DrawAs = ESlateBrushDrawType::Image;
		bChanged = true;
	}

	if (Fixed.GetTiling() != ESlateBrushTileType::NoTile)
	{
		UE_LOG(LogGlowImage, Warning, TEXT("GlowImage %s: Tiling mode %d is unsupported for glow tracking — use NoTile. Glow quad still follows widget bounds, but art may mismatch."),
			*GetName(), static_cast<int32>(Fixed.GetTiling()));
	}

	// Mirroring intentionally has no warning — flipped art still fills the same quad.
	// ImageSize is auto-synced below, so no warning for it either.
	// Any UTexture (2D, render target, dynamic) reports its size via the base
	// class; atlased resources report theirs via the atlas interface. Materials
	// have no pixel size and are left alone.
	if (UObject* Resource = Fixed.GetResourceObject())
	{
		FVector2f Desired = FVector2f::ZeroVector;
		bool bHasDesired = false;

		if (UTexture* Texture = Cast<UTexture>(Resource))
		{
			Desired = FVector2f(Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());
			bHasDesired = Desired.X > 0.f && Desired.Y > 0.f;
		}
		else if (ISlateTextureAtlasInterface* Atlas = Cast<ISlateTextureAtlasInterface>(Resource))
		{
			const FVector2D AtlasRawSize = Atlas->GetSlateAtlasData().GetSourceDimensions();
			const FIntPoint AtlasSize = FIntPoint(AtlasRawSize.X, AtlasRawSize.Y);
			Desired = FVector2f(static_cast<float>(AtlasSize.X), static_cast<float>(AtlasSize.Y));
			bHasDesired = AtlasSize.X > 0 && AtlasSize.Y > 0;
		}

		if (bHasDesired)
		{
			const FVector2f Existing = UE::Slate::CastToVector2f(Fixed.GetImageSize());
			if (!Existing.Equals(Desired))
			{
				Fixed.SetImageSize(Desired);
				bChanged = true;
			}
		}
	}

	if (bChanged)
	{
#if WITH_EDITOR
		const bool bWasModified = Modify();
		// Modify() can be refused (transient duplicates, save phase...). The
		// package flag is unconditional — without it the revert fixes only the
		// in-memory preview and re-fires every run even after saving.
		if (!GIsSavingPackage)
		{
			GetOutermost()->MarkPackageDirty();
		}
		UE_LOG(LogGlowImage, Display, TEXT("GlowImage %s brush fixup: obj=%s outer=%s flags[T=%d,A=%d,C=%d] modify=%d"),
			*GetName(), *GetPathName(), *GetOutermost()->GetName(),
			HasAnyFlags(RF_Transient) ? 1 : 0,
			HasAnyFlags(RF_ArchetypeObject) ? 1 : 0,
			HasAnyFlags(RF_ClassDefaultObject) ? 1 : 0,
			bWasModified ? 1 : 0);
#endif
		SetBrush(Fixed);
	}
}
