// Copyright Bohdon Sayre. All Rights Reserved.

using UnrealBuildTool;

public class WFC : ModuleRules
{
	public WFC(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"GameplayTags",
		});


		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"RHI",
			"Slate",
			"SlateCore",
		});
	}
}