// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_Zombie : ModuleRules
{
	public Project_Zombie(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "SlateCore",
            "GameplayTags",
			"DeveloperSettings",
			"NetCore" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// 현재 경로가 달라지면서 경고가 뜸.
		PublicIncludePaths.AddRange(new string[] {
			"Project_Zombie",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Platforming",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Platforming/Animation",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Combat",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Combat/AI",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Combat/Animation",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Combat/Gameplay",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Combat/Interfaces",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_Combat/UI",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_SideScrolling",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_SideScrolling/AI",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_SideScrolling/Gameplay",
			"Project_Zombie/EngineProvided_TPSTemplate/Variant_SideScrolling/Interfaces",
            "Project_Zombie/EngineProvided_TPSTemplate/Variant_SideScrolling/UI"
        });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
