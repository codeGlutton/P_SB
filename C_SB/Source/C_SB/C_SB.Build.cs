// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class C_SB : ModuleRules
{
	public C_SB(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "Sockets", "Networking", "EnhancedInput", "Niagara", "AIModule" , "NavigationSystem" , "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { "ProtobufCore" , "HTTP", "OnlineSubsystem" , "LoginFlow", "OnlineSubsystemUtils" , "Slate" , "SlateCore" });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            DynamicallyLoadedModuleNames.Add("OnlineSubsystemGoogle");
        }

        PrivateIncludePaths.AddRange(new string[] {
            "C_SB",
            "C_SB/Main",
            "C_SB/Game",
            "C_SB/Protocol",
            "C_SB/Utils",
            "C_SB/Network",
            "C_SB/Memory",
            "C_SB/Lock",
            "C_SB/Container",
            "C_SB/DataTable",
            "C_SB/UI"
        });
    }
}
