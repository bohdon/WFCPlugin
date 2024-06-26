﻿// Copyright Bohdon Sayre. All Rights Reserved.

using UnrealBuildTool;

public class WFCEditor : ModuleRules
{
	public WFCEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"UnrealEd",
			"WFC",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
		});
	}
}