using UnrealBuildTool;

public class StructChooserUncooked : ModuleRules
{
	public StructChooserUncooked(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"StructChooser",
			"Chooser",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"BlueprintGraph",
			"KismetCompiler",
			"Kismet",
			"GraphEditor",
		});
	}
}
