// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WFCPluginEditorTarget : TargetRules
{
	public WFCPluginEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "WFCPlugin" });
	}
}