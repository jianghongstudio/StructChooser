#pragma once

#include "CoreMinimal.h"
#include "IObjectChooser.h"
#include "StructUtils/InstancedStruct.h"
#include "StructChooserTypes.generated.h"

class UStructChooserTable;

USTRUCT(Meta = (Hidden))
struct STRUCTCHOOSER_API FStructChooserBase : public FObjectChooserBase
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_RetVal_OneParam(EIteratorStatus, FStructChooserIteratorCallback, const FInstancedStruct&);

	virtual UObject* ChooseObject(FChooserEvaluationContext& Context) const override { return nullptr; }

	virtual bool ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const
	{
		return false;
	}

	virtual EIteratorStatus ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const
	{
		FInstancedStruct Result;
		if (ChooseStruct(Context, Result) && Result.IsValid())
		{
			return Callback.Execute(Result);
		}
		return EIteratorStatus::Failed;
	}
};

// Hidden: keep out of official ObjectResult Add Row (engine MakeCreateResultMenu skips Hidden).
// StructChooser uses its own table editor with a Struct-only Add Row menu.
USTRUCT(DisplayName = "Struct", Meta = (Hidden, Category = "StructChooser", Tooltip = "A concrete struct instance returned when this row is selected."))
struct STRUCTCHOOSER_API FStructValueChooser : public FStructChooserBase
{
	GENERATED_BODY()

	virtual bool ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const override;
	virtual EIteratorStatus ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const override;
	virtual void GetDebugName(FString& OutDebugName) const override;

	/** Display name shown/edited in the Result column. */
	UPROPERTY(EditAnywhere, Category = "Parameters")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "Parameters", Meta = (StructTypeConst))
	FInstancedStruct Value;
};

USTRUCT(DisplayName = "Evaluate Struct Chooser", Meta = (Hidden, Category = "StructChooser", Tooltip = "Reference another StructChooserTable asset, evaluated at runtime if this row is selected."))
struct STRUCTCHOOSER_API FEvaluateStructChooser : public FStructChooserBase
{
	GENERATED_BODY()

	virtual bool ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const override;
	virtual EIteratorStatus ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const override;
	virtual void GetDebugName(FString& OutDebugName) const override;

#if WITH_EDITOR
	virtual UObject* GetReferencedObject() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Parameters")
	TObjectPtr<UStructChooserTable> Chooser;
};

USTRUCT(DisplayName = "Nested Struct Chooser", Meta = (Hidden, Category = "StructChooser", Tooltip = "Reference another StructChooserTable embedded in this asset, evaluated at runtime if this row is selected."))
struct STRUCTCHOOSER_API FNestedStructChooser : public FStructChooserBase
{
	GENERATED_BODY()

	virtual bool ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const override;
	virtual EIteratorStatus ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const override;
	virtual void GetDebugName(FString& OutDebugName) const override;

	UPROPERTY()
	TObjectPtr<UStructChooserTable> Chooser;
};
