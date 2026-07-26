#include "StructChooserTypes.h"
#include "StructChooserTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserTypes)

bool FStructValueChooser::ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const
{
	if (!Value.IsValid())
	{
		return false;
	}
	OutResult = Value;
	return true;
}

FObjectChooserBase::EIteratorStatus FStructValueChooser::ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const
{
	if (!Value.IsValid())
	{
		return EIteratorStatus::Continue;
	}
	const EIteratorStatus Status = Callback.Execute(Value);
	return Status == EIteratorStatus::Continue ? EIteratorStatus::ContinueWithOutputs : Status;
}

void FStructValueChooser::GetDebugName(FString& OutDebugName) const
{
	if (!Name.IsEmpty())
	{
		OutDebugName = Name;
		return;
	}
	OutDebugName = Value.GetScriptStruct() ? Value.GetScriptStruct()->GetName() : TEXT("InvalidStruct");
}

bool FEvaluateStructChooser::ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const
{
	return UStructChooserTable::EvaluateStructChooserFirst(Context, Chooser, OutResult);
}

FObjectChooserBase::EIteratorStatus FEvaluateStructChooser::ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const
{
	return UStructChooserTable::EvaluateStructChooser(Context, Chooser, Callback);
}

void FEvaluateStructChooser::GetDebugName(FString& OutDebugName) const
{
	OutDebugName = GetNameSafe(Chooser);
}

#if WITH_EDITOR
UObject* FEvaluateStructChooser::GetReferencedObject() const
{
	return Chooser;
}
#endif

bool FNestedStructChooser::ChooseStruct(FChooserEvaluationContext& Context, FInstancedStruct& OutResult) const
{
	return UStructChooserTable::EvaluateStructChooserFirst(Context, Chooser, OutResult);
}

FObjectChooserBase::EIteratorStatus FNestedStructChooser::ChooseMultiStruct(FChooserEvaluationContext& Context, FStructChooserIteratorCallback Callback) const
{
	return UStructChooserTable::EvaluateStructChooser(Context, Chooser, Callback);
}

void FNestedStructChooser::GetDebugName(FString& OutDebugName) const
{
	OutDebugName = GetNameSafe(Chooser);
}
