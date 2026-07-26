#include "StructChooserInitializer.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserInitializer)

UClass* FStructChooserInitializer::OverrideClass(UClass* Class) const
{
	return UStructChooserTable::StaticClass();
}

void FStructChooserInitializer::InitializeSignature(UChooserSignature* ChooserSignature) const
{
	UStructChooserTable* Table = Cast<UStructChooserTable>(ChooserSignature);
	if (!Table)
	{
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
