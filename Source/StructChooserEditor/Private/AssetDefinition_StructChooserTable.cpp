#include "AssetDefinition_StructChooserTable.h"
#include "AssetDefinitionRegistry.h"
#include "Chooser.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_StructChooserTable)

EAssetCommandResult UAssetDefinition_StructChooserTable::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	// FChooserTableEditor::CreateEditor is not exported from ChooserEditor.
	// Forward to the UChooserTable asset definition (same module) so the full table editor opens.
	if (const UAssetDefinition* ChooserDefinition =
		UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(UChooserTable::StaticClass()))
	{
		if (ChooserDefinition != this)
		{
			return ChooserDefinition->OpenAssets(OpenArgs);
		}
	}

	return Super::OpenAssets(OpenArgs);
}
