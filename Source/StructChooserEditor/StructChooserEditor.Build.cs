using UnrealBuildTool;
using System.IO;

public class StructChooserEditor : ModuleRules
{
	public StructChooserEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "TableEditor"));
		// ChooserTableEditorCommands / ChooserEditorStyle live in ChooserEditor Private (UE5.7).
		PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Plugins", "Chooser", "Source", "ChooserEditor", "Private"));

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
