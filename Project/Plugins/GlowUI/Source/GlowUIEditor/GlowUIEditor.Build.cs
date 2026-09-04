// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GlowUIEditor : ModuleRules
{
	public GlowUIEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine",
			"Slate", "SlateCore", "UMG",
			"GlowUI"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"PropertyEditor",
			"UnrealEd"
		});
	}
}
