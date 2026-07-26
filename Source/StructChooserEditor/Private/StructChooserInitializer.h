#pragma once

#include "ChooserInitializer.h"
#include "StructChooserInitializer.generated.h"

/** Create-dialog entry (via engine Chooser factory) that produces a UStructChooserTable. */
USTRUCT(DisplayName = "Struct Result Chooser", Meta = (ToolTip = "A ChooserTable subclass that returns a struct instance as its primary result. Supports Evaluate/Nested Struct Chooser rows. Use the Evaluate Struct Chooser Blueprint node."))
struct FStructChooserInitializer : public FChooserInitializer
{
	GENERATED_BODY()

	virtual UClass* OverrideClass(UClass* Class) const override;
	virtual void InitializeSignature(UChooserSignature* ChooserSignature) const override;

	UPROPERTY(EditAnywhere, Category = "Result", Meta = (AllowAbstract = "false"))
	TObjectPtr<UScriptStruct> OutputStructType;
};
