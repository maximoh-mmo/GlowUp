// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowUIEditorModule.h"
#include "GlowBrushStructCustomization.h"
#include "GlowImage.h"
#include "GlowImageDetailsCustomization.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FGlowUIEditorModule"

DEFINE_LOG_CATEGORY(LogGlowUIEditor);

void FGlowUIEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	PropertyEditorModule.RegisterCustomClassLayout(
		UGlowImage::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGlowImageDetailsCustomization::MakeInstance));

	// Scoped brush filter: the details panel prefers identifier matches over the
	// engine's base SlateBrush editor, so this works regardless of when the
	// DetailCustomizations module registers its editor.
	GlowBrushIdentifier = MakeShareable(new FGlowBrushIdentifier());
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		TEXT("SlateBrush"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGlowBrushStructCustomization::MakeInstance),
		GlowBrushIdentifier);
	UE_LOG(LogGlowUIEditor, Display, TEXT("GlowUIEditor loaded: GlowImage brush filter registered."));
}

void FGlowUIEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyEditorModule =
			FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

		PropertyEditorModule.UnregisterCustomClassLayout(UGlowImage::StaticClass()->GetFName());
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(TEXT("SlateBrush"), GlowBrushIdentifier);
	}

	GlowBrushIdentifier.Reset();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGlowUIEditorModule, GlowUIEditor)
