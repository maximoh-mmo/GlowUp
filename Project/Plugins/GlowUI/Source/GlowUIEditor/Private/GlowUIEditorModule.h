// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGlowUIEditor, Log, All);

class IPropertyTypeIdentifier;

class FGlowUIEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<IPropertyTypeIdentifier> GlowBrushIdentifier;
};
