// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyEditorModule.h"

// Matches only FSlateBrush structs owned by UGlowImage widgets. Registered
// alongside (not instead of) the engine's brush editor: the details panel
// prefers identifier matches and falls back to the engine editor otherwise,
// so load order vs the DetailCustomizations module does not matter.
class FGlowBrushIdentifier : public IPropertyTypeIdentifier
{
public:
	virtual ~FGlowBrushIdentifier() = default;
	virtual bool IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const override;
};

// Filtered brush UI for glow images: resource picker, tint and mirroring only.
// Draw As (forced Image), Tiling (forced NoTile) and Image Size (auto-synced
// from the texture in UGlowImage::EnsureBrushConstraints) are hidden; Margin
// and Outline Settings are inert in Image mode so they are hidden too.
class FGlowBrushStructCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
};
