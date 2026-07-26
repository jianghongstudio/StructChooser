using UnrealBuildTool;

public class StructChooserEditor : ModuleRules
{
	public StructChooserEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"StructChooser",
			"Chooser",
			"ChooserEditor",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"AssetDefinition",
			"AssetTools",
			"Slate",
			"SlateCore",
			"InputCore",
			"PropertyEditor",
			"EditorStyle",
			"EditorWidgets",
			"ToolWidgets",
			"ToolMenus",
			"StructUtilsEditor",
			"AssetRegistry",
			"ApplicationCore",
			"StructViewer",
		});
	}
}
