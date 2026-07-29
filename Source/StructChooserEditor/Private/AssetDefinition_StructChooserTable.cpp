#include "AssetDefinition_StructChooserTable.h"
#include "StructChooserTable.h"
#include "StructChooserTableEditor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_StructChooserTable)

EAssetCommandResult UAssetDefinition_StructChooserTable::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	TArray<UObject*> Objects = OpenArgs.LoadObjects<UObject>();

	for (UObject* Object : Objects)
	{
		UE::StructChooserEditor::FStructChooserTableEditor::CreateEditor(
			OpenArgs.GetToolkitMode(),
			OpenArgs.ToolkitHost,
			Object);
	}

	return EAssetCommandResult::Handled;
}
