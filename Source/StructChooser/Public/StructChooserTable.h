#pragma once

#include "CoreMinimal.h"
#include "Chooser.h"
#include "StructChooserTypes.h"
#include "StructChooserTable.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FStructChooserOutputStructTypeChanged, const UScriptStruct*);

/**
 * ChooserTable subclass whose primary result is an FInstancedStruct.
 * Use EvaluateStructChooser / the Evaluate Struct Chooser BP node — not the engine Object Evaluate Chooser.
 */
UCLASS(BlueprintType)
class STRUCTCHOOSER_API UStructChooserTable : public UChooserTable
{
	GENERATED_BODY()

public:
	UStructChooserTable();

	virtual void PostLoad() override;
	virtual void Compile(bool bForce = false) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	FStructChooserOutputStructTypeChanged OnOutputStructTypeChanged;
#endif

	/** Struct type returned as the primary result when a row is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Result", Meta = (AllowAbstract = "false"))
	TObjectPtr<UScriptStruct> OutputStructType;

	/** Ensure parent ResultType stays ObjectResult so the Chooser editor shows a Result column. */
	void ApplyStructChooserDefaults();

	static FObjectChooserBase::EIteratorStatus EvaluateStructChooser(
		FChooserEvaluationContext& Context,
		const UStructChooserTable* Chooser,
		FStructChooserBase::FStructChooserIteratorCallback Callback);

	static bool EvaluateStructChooserFirst(
		FChooserEvaluationContext& Context,
		const UStructChooserTable* Chooser,
		FInstancedStruct& OutResult);

	static void EvaluateStructChooserAll(
		FChooserEvaluationContext& Context,
		const UStructChooserTable* Chooser,
		TArray<FInstancedStruct>& OutResults);

private:
#if WITH_EDITOR
	void ValidateStructResults(FDataValidationContext* Context, bool& bHasErrors) const;
	bool DoesChildMatchOutputType(const UStructChooserTable* Child) const;
	/** Replace ObjectChooser rows that are not FStructChooserBase; returns true if anything changed. */
	bool SanitizeInvalidStructResults();
	void MakeDefaultStructValueResult(FInstancedStruct& OutResult) const;
#endif
};
