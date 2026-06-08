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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Project_Zombie",
			"Project_Zombie/Variant_Platforming",
			"Project_Zombie/Variant_Platforming/Animation",
			"Project_Zombie/Variant_Combat",
			"Project_Zombie/Variant_Combat/AI",
			"Project_Zombie/Variant_Combat/Animation",
			"Project_Zombie/Variant_Combat/Gameplay",
			"Project_Zombie/Variant_Combat/Interfaces",
			"Project_Zombie/Variant_Combat/UI",
			"Project_Zombie/Variant_SideScrolling",
			"Project_Zombie/Variant_SideScrolling/AI",
			"Project_Zombie/Variant_SideScrolling/Gameplay",
			"Project_Zombie/Variant_SideScrolling/Interfaces",
			"Project_Zombie/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
