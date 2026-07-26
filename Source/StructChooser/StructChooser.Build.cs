using UnrealBuildTool;

public class StructChooser : ModuleRules
{
	public StructChooser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Chooser",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayTags",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"Slate",
				"SlateCore",
			});
		}
	}
}
