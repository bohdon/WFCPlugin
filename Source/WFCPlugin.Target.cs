// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WFCPluginTarget : TargetRules
{
	public WFCPluginTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "WFCPlugin" });
	}
}