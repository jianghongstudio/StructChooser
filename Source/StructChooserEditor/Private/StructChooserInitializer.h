#pragma once

#include "ChooserInitializer.h"
#include "StructChooserInitializer.generated.h"

/**
 * Optional initializer for programmatic setup of a UStructChooserTable.
 * UE5.7 ChooserFactory has no OverrideClass — create assets via UStructChooserTableFactory
 * (Content Browser → Struct Chooser Table). Hidden so it does not appear in the engine
 * Chooser create-dialog type list (which can only spawn UChooserTable).
 */
USTRUCT(Meta = (Hidden))
struct FStructChooserInitializer : public FChooserInitializer
{
	GENERATED_BODY()

	virtual void Initialize(UChooserTable* Chooser) const override;

	UPROPERTY(EditAnywhere, Category = "Result", Meta = (AllowAbstract = "false"))
	TObjectPtr<UScriptStruct> OutputStructType;
};
