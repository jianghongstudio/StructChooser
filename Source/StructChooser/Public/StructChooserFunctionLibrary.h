#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructChooserTable.h"
#include "StructChooserFunctionLibrary.generated.h"

UCLASS()
class STRUCTCHOOSER_API UStructChooserFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Evaluate a StructChooser and return the first matching struct result. */
	UFUNCTION(BlueprintCallable, Category = "Struct Chooser", Meta = (BlueprintThreadSafe))
	static FInstancedStruct EvaluateStructChooser(const UObject* ContextObject, const UStructChooserTable* ChooserTable);

	/** Evaluate a StructChooser and return all matching struct results. */
	UFUNCTION(BlueprintCallable, Category = "Struct Chooser", Meta = (BlueprintThreadSafe))
	static TArray<FInstancedStruct> EvaluateStructChooserMulti(const UObject* ContextObject, const UStructChooserTable* ChooserTable);

	/** Evaluate using an existing evaluation context (for custom / K2 expand paths). */
	UFUNCTION(BlueprintCallable, Category = "Struct Chooser", Meta = (BlueprintThreadSafe))
	static FInstancedStruct EvaluateStructChooserWithContext(UPARAM(Ref) FChooserEvaluationContext& Context, const UStructChooserTable* ChooserTable);

	UFUNCTION(BlueprintCallable, Category = "Struct Chooser", Meta = (BlueprintThreadSafe))
	static TArray<FInstancedStruct> EvaluateStructChooserMultiWithContext(UPARAM(Ref) FChooserEvaluationContext& Context, const UStructChooserTable* ChooserTable);

	/**
	 * Typed evaluate used by UK2Node_EvaluateStructChooser.
	 * OutResult is a wildcard struct matching the table's OutputStructType.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "OutResult", BlueprintThreadSafe))
	static void EvaluateStructChooserTyped(const UObject* ContextObject, const UStructChooserTable* ChooserTable, int32& OutResult);

	DECLARE_FUNCTION(execEvaluateStructChooserTyped);

	UFUNCTION(BlueprintCallable, CustomThunk, Meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "OutResults", BlueprintThreadSafe))
	static void EvaluateStructChooserTypedMulti(const UObject* ContextObject, const UStructChooserTable* ChooserTable, TArray<int32>& OutResults);

	DECLARE_FUNCTION(execEvaluateStructChooserTypedMulti);

	UFUNCTION(BlueprintCallable, CustomThunk, Meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "OutResult", BlueprintThreadSafe))
	static void EvaluateStructChooserTypedWithContext(UPARAM(Ref) FChooserEvaluationContext& Context, const UStructChooserTable* ChooserTable, int32& OutResult);

	DECLARE_FUNCTION(execEvaluateStructChooserTypedWithContext);
};
