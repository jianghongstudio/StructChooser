using UnrealBuildTool;

public class StructChooserEditor : ModuleRules
{
	public StructChooserEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Private", "TableEditor"));

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
			"Persona",
			"BlueprintGraph",
			"GameplayTags",
			"GameplayTagsEditor",
			"DeveloperSettings",
			"GraphEditor",
		});
	}
}
