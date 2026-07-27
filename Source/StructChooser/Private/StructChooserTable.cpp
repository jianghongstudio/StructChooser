#include "StructChooserTable.h"
#include "ChooserIndexArray.h"
#include "ChooserPropertyAccess.h"
#include "ChooserTrace.h"
#include "IChooserColumn.h"
#include "Misc/DataValidation.h"
#include "Algo/Sort.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/TransactionObjectEvent.h"
#include "Styling/CoreStyle.h"
#include "UObject/ObjectSaveContext.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserTable)

DEFINE_LOG_CATEGORY_STATIC(LogStructChooser, Log, All);

UStructChooserTable::UStructChooserTable()
{
	ApplyStructChooserDefaults();
}

void UStructChooserTable::ApplyStructChooserDefaults()
{
	ResultType = EObjectChooserResultType::ObjectResult;
	if (!OutputObjectType)
	{
		OutputObjectType = UObject::StaticClass();
	}
}

void UStructChooserTable::PostLoad()
{
	Super::PostLoad();
	ApplyStructChooserDefaults();
#if WITH_EDITOR
	// No cell widgets yet — safe to fix bad serialized rows immediately.
	SanitizeInvalidStructResults();
#endif
}

void UStructChooserTable::Compile(bool bForce)
{
	Super::Compile(bForce);
	ApplyStructChooserDefaults();
}

#if WITH_EDITOR
void UStructChooserTable::MakeDefaultStructValueResult(FInstancedStruct& OutResult) const
{
	OutResult.InitializeAs(FStructValueChooser::StaticStruct());
	if (OutputStructType)
	{
		OutResult.GetMutable<FStructValueChooser>().Value.InitializeAs(OutputStructType);
	}
}

void UStructChooserTable::NotifyInvalidResultReplaced() const
{
	FNotificationInfo Info(NSLOCTEXT(
		"StructChooser",
		"InvalidResultReplaced",
		"StructChooser only supports Struct / Evaluate Struct Chooser / Nested Struct Chooser. Invalid Object result types were reset to Struct."));
	Info.ExpireDuration = 5.0f;
	Info.bUseSuccessFailIcons = true;
	Info.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
	FSlateNotificationManager::Get().AddNotification(Info);
}

void* UStructChooserTable::ReplaceInvalidResultAt(void* ValueMemory)
{
	if (!ValueMemory)
	{
		return nullptr;
	}

	auto TryReplace = [this, ValueMemory](FInstancedStruct& ResultData, const TCHAR* Where) -> void*
	{
		if (!ResultData.IsValid() || ResultData.GetMemory() != ValueMemory)
		{
			return nullptr;
		}
		if (ResultData.GetPtr<FStructChooserBase>() != nullptr)
		{
			return nullptr;
		}
		if (ResultData.GetPtr<FObjectChooserBase>() == nullptr)
		{
			return nullptr;
		}

		const FString OldType = GetNameSafe(ResultData.GetScriptStruct());
		MakeDefaultStructValueResult(ResultData);
		UE_LOG(LogStructChooser, Warning,
			TEXT("%s: Replaced invalid result type '%s' with Struct on '%s'."),
			Where,
			*OldType,
			*GetPathName());
		// InitializeAs may free the old buffer — callers must use this new pointer.
		return ResultData.GetMutableMemory();
	};

#if WITH_EDITORONLY_DATA
	for (int32 Index = 0; Index < ResultsStructs.Num(); ++Index)
	{
		if (void* NewMemory = TryReplace(ResultsStructs[Index], *FString::Printf(TEXT("Row %d"), Index)))
		{
			NotifyInvalidResultReplaced();
			return NewMemory;
		}
	}
#endif
	if (void* NewMemory = TryReplace(FallbackResult, TEXT("Fallback")))
	{
		NotifyInvalidResultReplaced();
		return NewMemory;
	}

	return nullptr;
}

bool UStructChooserTable::SanitizeInvalidStructResults()
{
	static bool bIsSanitizing = false;
	if (bIsSanitizing)
	{
		return false;
	}

	bool bChanged = false;

	auto SanitizeOne = [this, &bChanged](FInstancedStruct& ResultData, const TCHAR* Where)
	{
		if (!ResultData.IsValid())
		{
			return;
		}

		const bool bIsStructChooserResult = ResultData.GetPtr<FStructChooserBase>() != nullptr;
		const bool bIsObjectChooserResult = ResultData.GetPtr<FObjectChooserBase>() != nullptr;
		if (bIsObjectChooserResult && !bIsStructChooserResult)
		{
			const FString OldType = GetNameSafe(ResultData.GetScriptStruct());
			MakeDefaultStructValueResult(ResultData);
			bChanged = true;

			UE_LOG(LogStructChooser, Warning,
				TEXT("%s: Replaced invalid result type '%s' with Struct on '%s'. StructChooser only supports Struct / Evaluate Struct Chooser / Nested Struct Chooser."),
				Where,
				*OldType,
				*GetPathName());
		}
	};

	bIsSanitizing = true;

#if WITH_EDITORONLY_DATA
	for (int32 Index = 0; Index < ResultsStructs.Num(); ++Index)
	{
		SanitizeOne(ResultsStructs[Index], *FString::Printf(TEXT("Row %d"), Index));
	}
#endif
	SanitizeOne(FallbackResult, TEXT("Fallback"));

	bIsSanitizing = false;

	if (bChanged)
	{
		NotifyInvalidResultReplaced();
	}

	return bChanged;
}

void UStructChooserTable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UStructChooserTable, OutputStructType))
	{
		OnOutputStructTypeChanged.Broadcast(OutputStructType);
	}

	ApplyStructChooserDefaults();
	// Cell guards retype before building widgets; this catches Add Row / paste paths.
	SanitizeInvalidStructResults();
}

void UStructChooserTable::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);
	SanitizeInvalidStructResults();
}

void UStructChooserTable::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	SanitizeInvalidStructResults();
	Super::PreSave(ObjectSaveContext);
}

bool UStructChooserTable::DoesChildMatchOutputType(const UStructChooserTable* Child) const
{
	if (!Child)
	{
		return false;
	}
	const UStructChooserTable* ChildRoot = Cast<UStructChooserTable>(Child->GetRootChooser());
	const UScriptStruct* ChildType = ChildRoot ? ChildRoot->OutputStructType.Get() : Child->OutputStructType.Get();
	return ChildType != nullptr && ChildType == OutputStructType;
}

void UStructChooserTable::ValidateStructResults(FDataValidationContext* Context, bool& bHasErrors) const
{
	auto CheckResult = [this, Context, &bHasErrors](const FInstancedStruct& ResultData, const TCHAR* Where)
	{
		if (!ResultData.IsValid())
		{
			return;
		}

		if (const FStructValueChooser* ValueChooser = ResultData.GetPtr<FStructValueChooser>())
		{
			if (OutputStructType && ValueChooser->Value.IsValid() && ValueChooser->Value.GetScriptStruct() != OutputStructType)
			{
				bHasErrors = true;
				if (Context)
				{
					Context->AddError(FText::FromString(FString::Printf(
						TEXT("%s: Struct value type '%s' does not match OutputStructType '%s'."),
						Where,
						*GetNameSafe(ValueChooser->Value.GetScriptStruct()),
						*GetNameSafe(OutputStructType))));
				}
			}
		}
		else if (const FEvaluateStructChooser* Eval = ResultData.GetPtr<FEvaluateStructChooser>())
		{
			if (Eval->Chooser && !DoesChildMatchOutputType(Eval->Chooser))
			{
				bHasErrors = true;
				if (Context)
				{
					Context->AddError(FText::FromString(FString::Printf(
						TEXT("%s: Evaluate Struct Chooser '%s' OutputStructType mismatch."),
						Where,
						*GetNameSafe(Eval->Chooser))));
				}
			}
		}
		else if (const FNestedStructChooser* Nested = ResultData.GetPtr<FNestedStructChooser>())
		{
			if (Nested->Chooser && !DoesChildMatchOutputType(Nested->Chooser))
			{
				bHasErrors = true;
				if (Context)
				{
					Context->AddError(FText::FromString(FString::Printf(
						TEXT("%s: Nested Struct Chooser '%s' OutputStructType mismatch."),
						Where,
						*GetNameSafe(Nested->Chooser))));
				}
			}
		}
		else if (ResultData.GetPtr<FStructChooserBase>() == nullptr && ResultData.GetPtr<FObjectChooserBase>() != nullptr)
		{
			// Non-struct ObjectChooser row on a StructChooser table
			bHasErrors = true;
			if (Context)
			{
				Context->AddError(FText::FromString(FString::Printf(
					TEXT("%s: Row result '%s' is not a StructChooser result type. Use Struct / Evaluate Struct Chooser / Nested Struct Chooser."),
					Where,
					*GetNameSafe(ResultData.GetScriptStruct()))));
			}
		}
	};

#if WITH_EDITORONLY_DATA
	for (int32 Index = 0; Index < ResultsStructs.Num(); ++Index)
	{
		CheckResult(ResultsStructs[Index], *FString::Printf(TEXT("Row %d"), Index));
	}
#endif
	CheckResult(FallbackResult, TEXT("Fallback"));
}

EDataValidationResult UStructChooserTable::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!OutputStructType)
	{
		Context.AddError(NSLOCTEXT("StructChooser", "MissingOutputStructType", "OutputStructType must be set on a StructChooserTable."));
		Result = EDataValidationResult::Invalid;
	}

	bool bHasErrors = false;
	ValidateStructResults(&Context, bHasErrors);
	if (bHasErrors)
	{
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

FObjectChooserBase::EIteratorStatus UStructChooserTable::EvaluateStructChooser(
	FChooserEvaluationContext& Context,
	const UStructChooserTable* Chooser,
	FStructChooserBase::FStructChooserIteratorCallback Callback)
{
	if (Chooser == nullptr)
	{
		return FObjectChooserBase::EIteratorStatus::Continue;
	}

	VALIDATE_CHOOSER_CONTEXT(Chooser, Chooser->ContextData, Context);

#if WITH_EDITOR
	Chooser->UpdateDebugging(Context);
#endif

#if CHOOSER_DEBUGGING_ENABLED
	Context.DebuggingInfo.CurrentChooser = Chooser;
#endif

	const TArray<FInstancedStruct>* ResultsArray = &Chooser->CookedResults;

#if WITH_EDITORONLY_DATA
	if (!Chooser->IsCookedData())
	{
		ResultsArray = &Chooser->ResultsStructs;
	}
#endif

	const uint32 Count = ResultsArray->Num();
	const uint32 BufferSize = Count * sizeof(FChooserIndexArray::FIndexData);

	FChooserIndexArray Indices1(static_cast<FChooserIndexArray::FIndexData*>(FMemory_Alloca(BufferSize)), Count);
	FChooserIndexArray Indices2(static_cast<FChooserIndexArray::FIndexData*>(FMemory_Alloca(BufferSize)), Count);

	for (uint32 i = 0; i < Count; i++)
	{
		if (!Chooser->IsRowDisabled(i))
		{
			Indices1.Push({i, 0});
		}
	}

	static constexpr int32 ScratchAreaAlignment = 16;
	int32 TotalScratchAreaSize = 0;
	for (const FInstancedStruct& ColumnData : Chooser->ColumnsStructs)
	{
		const FChooserColumnBase& Column = ColumnData.Get<FChooserColumnBase>();
		TotalScratchAreaSize += Align(Column.GetScratchAreaSize(), ScratchAreaAlignment);
	}
	TArrayView<uint8> ScratchArea(
		static_cast<uint8*>(FMemory_Alloca_Aligned(TotalScratchAreaSize * sizeof(uint8), ScratchAreaAlignment)),
		TotalScratchAreaSize);

	auto DeinitializeScratchAreas = [Chooser, ScratchArea, TotalScratchAreaSize]()
	{
		if (TotalScratchAreaSize <= 0)
		{
			return;
		}
		int32 ScratchAreaStart = 0;
		for (const FInstancedStruct& ColumnData : Chooser->ColumnsStructs)
		{
			const FChooserColumnBase& Column = ColumnData.Get<FChooserColumnBase>();
			const int32 ColumnScratchAreaSize = Column.GetScratchAreaSize();
			TArrayView<uint8> ColumnScratchArea = ScratchArea.Slice(ScratchAreaStart, ColumnScratchAreaSize);
			Column.DeinitializeScratchArea(ColumnScratchArea);
			ScratchAreaStart += Align(ColumnScratchAreaSize, ScratchAreaAlignment);
		}
	};

	FChooserIndexArray* IndicesOut = &Indices1;
	FChooserIndexArray* IndicesIn = &Indices2;

	int32 ScratchAreaStart = 0;
	for (const FInstancedStruct& ColumnData : Chooser->ColumnsStructs)
	{
		const FChooserColumnBase& Column = ColumnData.Get<FChooserColumnBase>();
		const int32 ColumnScratchAreaSize = Column.GetScratchAreaSize();
		TArrayView<uint8> ColumnScratchArea = ScratchArea.Slice(ScratchAreaStart, ColumnScratchAreaSize);
		Column.InitializeScratchArea(ColumnScratchArea);
		ScratchAreaStart += Align(ColumnScratchAreaSize, ScratchAreaAlignment);

#if WITH_EDITORONLY_DATA
		if (Column.bDisabled)
		{
			continue;
		}
#endif

		if (Column.HasFilters())
		{
			Swap(IndicesIn, IndicesOut);
			IndicesOut->SetNum(0);
			Column.Filter(Context, *IndicesIn, *IndicesOut, ColumnScratchArea);

			if (IndicesIn->HasCosts() || Column.HasCosts())
			{
				IndicesOut->SetHasCosts();
			}
		}
	}

	if (IndicesOut->Num() > 1 && IndicesOut->HasCosts())
	{
		Algo::Sort(*IndicesOut);
	}

	bool bAnyRowSucceeded = false;

	for (const FChooserIndexArray::FIndexData& SelectedIndexData : *IndicesOut)
	{
		if (!ResultsArray->IsValidIndex(SelectedIndexData.Index))
		{
			continue;
		}

		ScratchAreaStart = 0;
		for (const FInstancedStruct& ColumnData : Chooser->ColumnsStructs)
		{
			const FChooserColumnBase& Column = ColumnData.Get<FChooserColumnBase>();
			const int32 ColumnScratchAreaSize = Column.GetScratchAreaSize();
			TArrayView<uint8> ColumnScratchArea = ScratchArea.Slice(ScratchAreaStart, ColumnScratchAreaSize);
			ScratchAreaStart += Align(ColumnScratchAreaSize, ScratchAreaAlignment);

#if WITH_EDITORONLY_DATA
			if (Column.bDisabled)
			{
				continue;
			}
#endif
			Column.SetOutputs(Context, SelectedIndexData.Index, ColumnScratchArea);
		}

		if (const FStructChooserBase* SelectedResult = (*ResultsArray)[SelectedIndexData.Index].GetPtr<FStructChooserBase>())
		{
			const FObjectChooserBase::EIteratorStatus Status = SelectedResult->ChooseMultiStruct(Context, Callback);
			// UE5.7: Continue == no hit; ContinueWithOutputs / Stop == produced a result.
			if (Status != FObjectChooserBase::EIteratorStatus::Continue)
			{
				bAnyRowSucceeded = true;
#if WITH_EDITOR
				if (Context.DebuggingInfo.bCurrentDebugTarget)
				{
					Chooser->SetDebugSelectedRow(SelectedIndexData.Index);
				}
#endif
				// Emits ChooserChannel events for Rewind Debugger "Chooser Evaluation" tracks.
				TRACE_CHOOSER_EVALUATION(Chooser, Context, SelectedIndexData.Index);
			}

			if (Status == FObjectChooserBase::EIteratorStatus::Stop)
			{
				DeinitializeScratchAreas();
				return FObjectChooserBase::EIteratorStatus::Stop;
			}
		}
	}

	if (!bAnyRowSucceeded)
	{
#if WITH_EDITOR
		if (Context.DebuggingInfo.bCurrentDebugTarget)
		{
			Chooser->SetDebugSelectedRow(ChooserColumn_SpecialIndex_Fallback);
		}
#endif
		TRACE_CHOOSER_EVALUATION(Chooser, Context, ChooserColumn_SpecialIndex_Fallback);

		if (Chooser->FallbackResult.IsValid())
		{
			if (const FStructChooserBase* SelectedResult = Chooser->FallbackResult.GetPtr<FStructChooserBase>())
			{
				const FObjectChooserBase::EIteratorStatus Status = SelectedResult->ChooseMultiStruct(Context, Callback);
				if (Status != FObjectChooserBase::EIteratorStatus::Continue)
				{
					bAnyRowSucceeded = true;
					ScratchAreaStart = 0;
					for (const FInstancedStruct& ColumnData : Chooser->ColumnsStructs)
					{
						const FChooserColumnBase& Column = ColumnData.Get<FChooserColumnBase>();
						const int32 ColumnScratchAreaSize = Column.GetScratchAreaSize();
						TArrayView<uint8> ColumnScratchArea = ScratchArea.Slice(ScratchAreaStart, ColumnScratchAreaSize);
						ScratchAreaStart += Align(ColumnScratchAreaSize, ScratchAreaAlignment);
						Column.SetOutputs(Context, ChooserColumn_SpecialIndex_Fallback, ColumnScratchArea);
					}
				}
				if (Status == FObjectChooserBase::EIteratorStatus::Stop)
				{
					DeinitializeScratchAreas();
					return FObjectChooserBase::EIteratorStatus::Stop;
				}
			}
		}
	}

	DeinitializeScratchAreas();
	return bAnyRowSucceeded
		? FObjectChooserBase::EIteratorStatus::ContinueWithOutputs
		: FObjectChooserBase::EIteratorStatus::Continue;
}

bool UStructChooserTable::EvaluateStructChooserFirst(
	FChooserEvaluationContext& Context,
	const UStructChooserTable* Chooser,
	FInstancedStruct& OutResult)
{
	bool bFound = false;
	EvaluateStructChooser(Context, Chooser, FStructChooserBase::FStructChooserIteratorCallback::CreateLambda(
		[&OutResult, &bFound](const FInstancedStruct& InResult)
		{
			OutResult = InResult;
			bFound = true;
			return FObjectChooserBase::EIteratorStatus::Stop;
		}));
	return bFound;
}

void UStructChooserTable::EvaluateStructChooserAll(
	FChooserEvaluationContext& Context,
	const UStructChooserTable* Chooser,
	TArray<FInstancedStruct>& OutResults)
{
	OutResults.Reset();
	EvaluateStructChooser(Context, Chooser, FStructChooserBase::FStructChooserIteratorCallback::CreateLambda(
		[&OutResults](const FInstancedStruct& InResult)
		{
			OutResults.Add(InResult);
			return FObjectChooserBase::EIteratorStatus::Continue;
		}));
}
