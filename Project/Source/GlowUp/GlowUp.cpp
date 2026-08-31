// Copyright Epic Games, Inc. All Rights Reserved.

#include "GlowUp.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Modules/ModuleManager.h"
#include "Layout/Geometry.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, GlowUp, "GlowUp" );

void USlateBlueprintLibrary::AbsoluteToViewport(const UObject* WorldContextObject, FVector2D AbsoluteDesktopCoordinate, FVector2D& PixelPosition, FVector2D& ViewportPosition)
{
	
}
