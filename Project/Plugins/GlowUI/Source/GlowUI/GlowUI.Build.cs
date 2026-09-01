// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GlowUI : ModuleRules
{
	public GlowUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "UMG", "Slate", "SlateCore"
		});
	}
}
