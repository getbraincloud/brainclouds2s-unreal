// Copyright Epic Games, Inc. All Rights Reserved.

using System.Collections.Generic;
using System;
using System.IO;
using UnrealBuildTool;

public class BrainCloudS2SPlugin : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    public BrainCloudS2SPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "Private/BrainCloudS2SPrivatePCH.h";
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
            new string[] {
                    Path.Combine(ModulePath,"Private")
                });

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "InputCore", 
				"WebSockets",
				"BCClientPlugin"
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "JsonUtilities", 
				"HTTP", 
				"Json"
			}
			);
	}
}
