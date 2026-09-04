// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "GlowImage.generated.h"

class USlateBrushAsset;

/**
 * 
 */
DECLARE_LOG_CATEGORY_EXTERN(LogGlowImage, Display,Log)
UCLASS()
class GLOWUI_API UGlowImage : public UImage
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Glow" )
	FLinearColor GlowColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Glow" )
	float GlowIntensity = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Glow" )
	bool bGlowEnabled = true;
	
	UPROPERTY(EditAnywhere, Category = "Appearance|Glow" )
	TObjectPtr<UMaterialInterface> GlowMaterialParent;

	
protected:
	// Primary initialization — call from Blueprint or automatically from OnInitialized/RebuildWidget
	UFUNCTION(BlueprintCallable, Category = "Appearance|Glow" )
	void InitializeGlow();
	
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
	void RegisterWithSubsystem();
	void EnsureBrushConstraints();
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GlowDMI;
};
