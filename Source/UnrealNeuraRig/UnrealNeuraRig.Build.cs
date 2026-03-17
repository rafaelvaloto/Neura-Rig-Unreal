// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class UnrealNeuraRig : ModuleRules
{
	public UnrealNeuraRig(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",
				"Networking",
				"Sockets",
				"ControlRig", // Essencial para as Units
				"RigVM"       // Essencial para o Grafo
			});
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			});
	}
}
