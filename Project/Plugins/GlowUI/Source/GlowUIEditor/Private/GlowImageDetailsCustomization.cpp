// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowImageDetailsCustomization.h"
#include "Components/Image.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "GlowUIEditorModule.h"
#include "PropertyHandle.h"
#include "Styling/SlateBrush.h"

TSharedRef<IDetailCustomization> FGlowImageDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FGlowImageDetailsCustomization());
}

void FGlowImageDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Brush is declared on UImage (not UGlowImage), so the declaring class must
	// be passed explicitly — otherwise the lookup searches the wrong owner map
	// and returns an invalid handle.
	TSharedRef<IPropertyHandle> BrushHandle = DetailBuilder.GetProperty(FName(TEXT("Brush")), UImage::StaticClass());
	if (!BrushHandle->IsValidHandle())
	{
		UE_LOG(LogGlowUIEditor, Display, TEXT("GlowImage details: Brush handle invalid, leaving default UI."));
		return;
	}

	// Keep only the image picker; everything else (Image Size, Tint, Draw As,
	// Tiling, Margin, Outline, Mirroring) is glow-incompatible and stays hidden.
	// Nothing under Localization is touched.
	static const FName WantedChildren[] = {
		FName(TEXT("ResourceObject")),
	};

	TArray<TSharedPtr<IPropertyHandle>> ChildHandles;
	TSharedPtr<IPropertyHandle> ResourceHandle;
	for (const FName& ChildName : WantedChildren)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = BrushHandle->GetChildHandle(ChildName);
		if (!ChildHandle.IsValid() || !ChildHandle->IsValidHandle())
		{
			UE_LOG(LogGlowUIEditor, Display, TEXT("GlowImage details: brush child %s invalid, leaving default UI."), *ChildName.ToString());
			return;
		}
		if (ChildName == FName(TEXT("ResourceObject")))
		{
			ResourceHandle = ChildHandle;
		}
		ChildHandles.Add(ChildHandle);
	}

	// The default resource picker does not invalidate the brush's cached Slate
	// resource (the engine's brush editor does this in its custom widget), so
	// without this the designer keeps drawing the previous image.
	ResourceHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([ResourceHandle]()
	{
		if (!ResourceHandle.IsValid())
		{
			return;
		}
		TSharedPtr<IPropertyHandle> BrushParent = ResourceHandle->GetParentHandle();
		if (!BrushParent.IsValid())
		{
			return;
		}
		TArray<void*> RawBrushes;
		BrushParent->AccessRawData(RawBrushes);
		for (void* RawBrush : RawBrushes)
		{
			if (FSlateBrush* Brush = static_cast<FSlateBrush*>(RawBrush))
			{
				Brush->InvalidateResourceHandle();
			}
		}
	}));

	// Hiding the whole struct row also suppresses FSlateBrush's engine rows,
	// which only materialize under an active struct row.
	DetailBuilder.HideProperty(BrushHandle);

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Appearance"));
	for (const TSharedPtr<IPropertyHandle>& ChildHandle : ChildHandles)
	{
		Category.AddProperty(ChildHandle);
	}

	UE_LOG(LogGlowUIEditor, Display, TEXT("GlowImage details: brush restructured to Image picker only."));

	// Hide Color/Opacity (inherited from UImage) — glow tint comes from
	// Appearance|Glow instead. Blueprint access is unaffected.
	TSharedRef<IPropertyHandle> ColorHandle = DetailBuilder.GetProperty(FName(TEXT("ColorAndOpacity")), UImage::StaticClass());
	if (ColorHandle->IsValidHandle())
	{
		DetailBuilder.HideProperty(ColorHandle);
	}
	else
	{
		UE_LOG(LogGlowUIEditor, Display, TEXT("GlowImage details: ColorAndOpacity handle invalid, left visible."));
	}
}
