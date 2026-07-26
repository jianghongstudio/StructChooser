#include "StructChooserFunctionLibrary.h"
#include "Blueprint/BlueprintExceptionInfo.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserFunctionLibrary)

FInstancedStruct UStructChooserFunctionLibrary::EvaluateStructChooser(const UObject* ContextObject, const UStructChooserTable* ChooserTable)
{
	FChooserEvaluationContext Context(const_cast<UObject*>(ContextObject));
	return EvaluateStructChooserWithContext(Context, ChooserTable);
}

TArray<FInstancedStruct> UStructChooserFunctionLibrary::EvaluateStructChooserMulti(const UObject* ContextObject, const UStructChooserTable* ChooserTable)
{
	FChooserEvaluationContext Context(const_cast<UObject*>(ContextObject));
	return EvaluateStructChooserMultiWithContext(Context, ChooserTable);
}

FInstancedStruct UStructChooserFunctionLibrary::EvaluateStructChooserWithContext(FChooserEvaluationContext& Context, const UStructChooserTable* ChooserTable)
{
	FInstancedStruct Result;
	UStructChooserTable::EvaluateStructChooserFirst(Context, ChooserTable, Result);
	return Result;
}

TArray<FInstancedStruct> UStructChooserFunctionLibrary::EvaluateStructChooserMultiWithContext(FChooserEvaluationContext& Context, const UStructChooserTable* ChooserTable)
{
	TArray<FInstancedStruct> Results;
	UStructChooserTable::EvaluateStructChooserAll(Context, ChooserTable, Results);
	return Results;
}

void UStructChooserFunctionLibrary::EvaluateStructChooserTyped(const UObject* ContextObject, const UStructChooserTable* ChooserTable, int32& OutResult)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UStructChooserFunctionLibrary::execEvaluateStructChooserTyped)
{
	P_GET_OBJECT(UObject, ContextObject);
	P_GET_OBJECT(UStructChooserTable, ChooserTable);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			NSLOCTEXT("StructChooser", "TypedEval_InvalidOut", "Failed to resolve OutResult for EvaluateStructChooserTyped"));
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		return;
	}

	P_NATIVE_BEGIN;
	FChooserEvaluationContext EvalContext(ContextObject);
	FInstancedStruct Result;
	if (UStructChooserTable::EvaluateStructChooserFirst(EvalContext, ChooserTable, Result)
		&& Result.IsValid()
		&& Result.GetScriptStruct()->IsChildOf(ValueProp->Struct))
	{
		ValueProp->Struct->CopyScriptStruct(ValuePtr, Result.GetMemory());
	}
	else
	{
		ValueProp->Struct->ClearScriptStruct(ValuePtr);
	}
	P_NATIVE_END;
}

void UStructChooserFunctionLibrary::EvaluateStructChooserTypedMulti(const UObject* ContextObject, const UStructChooserTable* ChooserTable, TArray<int32>& OutResults)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UStructChooserFunctionLibrary::execEvaluateStructChooserTypedMulti)
{
	P_GET_OBJECT(UObject, ContextObject);
	P_GET_OBJECT(UStructChooserTable, ChooserTable);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FArrayProperty>(nullptr);

	const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Stack.MostRecentProperty);
	void* ArrayPtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!ArrayProp || !ArrayPtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			NSLOCTEXT("StructChooser", "TypedEvalMulti_InvalidOut", "Failed to resolve OutResults for EvaluateStructChooserTypedMulti"));
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		return;
	}

	P_NATIVE_BEGIN;
	const FStructProperty* StructProperty = CastField<FStructProperty>(ArrayProp->Inner);
	if (StructProperty)
	{
		FChooserEvaluationContext EvalContext(ContextObject);
		TArray<FInstancedStruct> Results;
		UStructChooserTable::EvaluateStructChooserAll(EvalContext, ChooserTable, Results);

		FScriptArrayHelper ArrayHelper(ArrayProp, ArrayPtr);
		ArrayHelper.EmptyAndAddValues(Results.Num());
		for (int32 Index = 0; Index < Results.Num(); ++Index)
		{
			if (Results[Index].IsValid() && Results[Index].GetScriptStruct()->IsChildOf(StructProperty->Struct))
			{
				StructProperty->Struct->CopyScriptStruct(ArrayHelper.GetRawPtr(Index), Results[Index].GetMemory());
			}
		}
	}
	P_NATIVE_END;
}

void UStructChooserFunctionLibrary::EvaluateStructChooserTypedWithContext(FChooserEvaluationContext& Context, const UStructChooserTable* ChooserTable, int32& OutResult)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UStructChooserFunctionLibrary::execEvaluateStructChooserTypedWithContext)
{
	P_GET_STRUCT_REF(FChooserEvaluationContext, EvalContext);
	P_GET_OBJECT(UStructChooserTable, ChooserTable);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			NSLOCTEXT("StructChooser", "TypedEvalCtx_InvalidOut", "Failed to resolve OutResult for EvaluateStructChooserTypedWithContext"));
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		return;
	}

	P_NATIVE_BEGIN;
	FInstancedStruct Result;
	if (UStructChooserTable::EvaluateStructChooserFirst(EvalContext, ChooserTable, Result)
		&& Result.IsValid()
		&& Result.GetScriptStruct()->IsChildOf(ValueProp->Struct))
	{
		ValueProp->Struct->CopyScriptStruct(ValuePtr, Result.GetMemory());
	}
	else
	{
		ValueProp->Struct->ClearScriptStruct(ValuePtr);
	}
	P_NATIVE_END;
}
