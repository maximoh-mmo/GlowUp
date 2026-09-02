// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "GlowImage.generated.h"

/**
 * 
 */

DECLARE_LOG_CATEGORY_EXTERN(LogGlowImage, Display,Log)
UCLASS()
class GLOWUI_API UGlowImage : public UImage
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glow")
	FLinearColor GlowColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glow")
	float GlowIntensity = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glow")
	bool bGlowEnabled = true;
	
	// Assign your validated PP material asset here or set a project default later
	UPROPERTY(EditAnywhere, Category = "Glow")
	TObjectPtr<UMaterialInterface> GlowMaterialParent;
	
protected:
	// Primary initialization — call from Blueprint or automatically from OnInitialized/RebuildWidget
	UFUNCTION(BlueprintCallable, Category = "Glow")
	void InitializeGlow();
	
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
	void RegisterWithSubsystem();
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GlowDMI;
};
