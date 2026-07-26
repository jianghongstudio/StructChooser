#include "Misc/AutomationTest.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"
#include "StructChooserFunctionLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStructChooserEvaluateNestedTest,
	"StructChooser.Evaluate.NestedAndDirect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStructChooserEvaluateNestedTest::RunTest(const FString& Parameters)
{
	// Use a simple engine struct as the result payload
	UScriptStruct* ResultType = TBaseStructure<FVector>::Get();
	TestNotNull(TEXT("ResultType"), ResultType);

	UStructChooserTable* Root = NewObject<UStructChooserTable>(GetTransientPackage());
	Root->ApplyStructChooserDefaults();
	Root->OutputStructType = ResultType;

	UStructChooserTable* Nested = NewObject<UStructChooserTable>(Root, TEXT("NestedChild"), RF_Transactional);
	Nested->ApplyStructChooserDefaults();
	Nested->OutputStructType = ResultType;
	Nested->RootChooser = Root;
	Root->AddNestedObject(Nested);

#if WITH_EDITORONLY_DATA
	{
		FInstancedStruct& NestedRow = Nested->ResultsStructs.AddDefaulted_GetRef();
		NestedRow.InitializeAs(FStructValueChooser::StaticStruct());
		FStructValueChooser& Value = NestedRow.GetMutable<FStructValueChooser>();
		Value.Value.InitializeAs(ResultType);
		Value.Value.GetMutable<FVector>() = FVector(1.0, 2.0, 3.0);
	}

	{
		FInstancedStruct& RootRow = Root->ResultsStructs.AddDefaulted_GetRef();
		RootRow.InitializeAs(FNestedStructChooser::StaticStruct());
		RootRow.GetMutable<FNestedStructChooser>().Chooser = Nested;
	}
#endif

	FChooserEvaluationContext Context;
	FInstancedStruct Out;
	const bool bOk = UStructChooserTable::EvaluateStructChooserFirst(Context, Root, Out);
	TestTrue(TEXT("Evaluate succeeded"), bOk);
	TestTrue(TEXT("Result valid"), Out.IsValid());
	TestTrue(TEXT("Result type"), Out.GetScriptStruct() == ResultType);
	if (Out.IsValid())
	{
		TestEqual(TEXT("Result value"), Out.Get<FVector>(), FVector(1.0, 2.0, 3.0));
	}

	// Engine Object path must not return a UObject for struct rows
	UObject* ObjectResult = nullptr;
	UChooserTable::EvaluateChooser(Context, Root, FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda(
		[&ObjectResult](UObject* InResult)
		{
			ObjectResult = InResult;
			return FObjectChooserBase::EIteratorStatus::Stop;
		}));
	TestNull(TEXT("Engine EvaluateChooser must not yield an object from StructChooser rows"), ObjectResult);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStructChooserEvaluateAssetRefTest,
	"StructChooser.Evaluate.EvaluateStructChooserRef",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStructChooserEvaluateAssetRefTest::RunTest(const FString& Parameters)
{
	UScriptStruct* ResultType = TBaseStructure<FVector>::Get();

	UStructChooserTable* Child = NewObject<UStructChooserTable>(GetTransientPackage());
	Child->ApplyStructChooserDefaults();
	Child->OutputStructType = ResultType;

	UStructChooserTable* Parent = NewObject<UStructChooserTable>(GetTransientPackage());
	Parent->ApplyStructChooserDefaults();
	Parent->OutputStructType = ResultType;

#if WITH_EDITORONLY_DATA
	{
		FInstancedStruct& ChildRow = Child->ResultsStructs.AddDefaulted_GetRef();
		ChildRow.InitializeAs(FStructValueChooser::StaticStruct());
		FStructValueChooser& Value = ChildRow.GetMutable<FStructValueChooser>();
		Value.Value.InitializeAs(ResultType);
		Value.Value.GetMutable<FVector>() = FVector(9.0, 8.0, 7.0);
	}
	{
		FInstancedStruct& ParentRow = Parent->ResultsStructs.AddDefaulted_GetRef();
		ParentRow.InitializeAs(FEvaluateStructChooser::StaticStruct());
		ParentRow.GetMutable<FEvaluateStructChooser>().Chooser = Child;
	}
#endif

	FInstancedStruct Result = UStructChooserFunctionLibrary::EvaluateStructChooser(nullptr, Parent);
	TestTrue(TEXT("Result valid"), Result.IsValid());
	if (Result.IsValid())
	{
		TestEqual(TEXT("Result value"), Result.Get<FVector>(), FVector(9.0, 8.0, 7.0));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
