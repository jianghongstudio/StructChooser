#include "StructChooserInitializer.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserInitializer)

void FStructChooserInitializer::Initialize(UChooserTable* Chooser) const
{
	UStructChooserTable* Table = Cast<UStructChooserTable>(Chooser);
	if (!Table)
	{
		// UE5.7 factory always creates UChooserTable; StructChooser assets must use UStructChooserTableFactory.
		return;
	}

	Table->ApplyStructChooserDefaults();
	Table->OutputStructType = OutputStructType;

#if WITH_EDITORONLY_DATA
	if (Table->ResultsStructs.IsEmpty())
	{
		FInstancedStruct& FirstRow = Table->ResultsStructs.AddDefaulted_GetRef();
		FirstRow.InitializeAs(FStructValueChooser::StaticStruct());
		if (OutputStructType)
		{
			FirstRow.GetMutable<FStructValueChooser>().Value.InitializeAs(OutputStructType);
		}
	}
#endif
}
