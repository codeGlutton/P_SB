// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OnlineSubsystemGoogleEx : ModuleRules
{
    public OnlineSubsystemGoogleEx(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreOnline",
                "CoreUObject",
                "ApplicationCore",
                "HTTP",
                "ImageCore",
                "Json",
                "Sockets",
                "OnlineSubsystem"
            }
            );
    }
}
