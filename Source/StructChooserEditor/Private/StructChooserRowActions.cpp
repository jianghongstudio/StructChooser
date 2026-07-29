#include "StructChooserRowActions.h"

#include "ScopedTransaction.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"

#define LOCTEXT_NAMESPACE "StructChooserRowActions"

namespace UE::StructChooserEditor
{
namespace
{
	TMap<TWeakObjectPtr<UStructChooserTable>, TWeakPtr<UE::ChooserEditor::IChooserTableViewModel>> GViewModelByRoot;

	void InitializeStructValueIfNeeded(FInstancedStruct& NewResult, UStructChooserTable* Table, const UScriptStruct* ResultType)
	{
		if (ResultType != FStructValueChooser::StaticStruct())
		{
			NewResult.InitializeAs(ResultType);
			return;
		}

		NewResult.InitializeAs(FStructValueChooser::StaticStruct());
		UScriptStruct* OutputType = nullptr;
		if (UStructChooserTable* Root = Cast<UStructChooserTable>(Table->GetRootChooser()))
		{
			OutputType = Root->OutputStructType;
		}
		else
		{
			OutputType = Table->OutputStructType;
		}
		if (OutputType)
		{
			NewResult.GetMutable<FStructValueChooser>().Value.InitializeAs(OutputType);
		}
	}

	void AlignDisabledRows(UStructChooserTable* Table)
	{
		while (Table->DisabledRows.Num() < Table->ResultsStructs.Num())
		{
			Table->DisabledRows.Add(false);
		}
	}

	void RefreshViewModel(TWeakPtr<UE::ChooserEditor::IChooserTableViewModel> WeakViewModel, UStructChooserTable* Table)
	{
		if (TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> VM = WeakViewModel.Pin())
		{
			VM->RefreshAll();
			return;
		}
		if (TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> Cached = FindCachedStructChooserViewModel(Table))
		{
			Cached->RefreshAll();
		}
	}

	void AppendStructResultTypeEntries(
		FMenuBuilder& MenuBuilder,
		TWeakObjectPtr<UStructChooserTable> WeakTable,
		TWeakPtr<UE::ChooserEditor::IChooserTableViewModel> WeakViewModel,
		bool bFallback)
	{
		auto AddEntry = [&](const FText& Label, const FText& Tooltip, UScriptStruct* Type)
		{
			MenuBuilder.AddMenuEntry(
				Label,
				Tooltip,
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([WeakTable, WeakViewModel, Type, bFallback]()
				{
					UStructChooserTable* Table = WeakTable.Get();
					if (!Table)
					{
						if (TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> VM = WeakViewModel.Pin())
						{
							Table = Cast<UStructChooserTable>(VM->GetChooser());
						}
					}
					if (!Table)
					{
						return;
					}

					if (bFallback)
					{
						AddStructFallbackResult(Table, Type);
					}
					else
					{
						AddStructResultRow(Table, Type);
					}
					RefreshViewModel(WeakViewModel, Table);
				})));
		};

		AddEntry(
			LOCTEXT("Struct", "Struct"),
			LOCTEXT("StructTip", "A concrete struct instance returned when this row is selected."),
			FStructValueChooser::StaticStruct());
		AddEntry(
			LOCTEXT("EvaluateStruct", "Evaluate Struct Chooser"),
			LOCTEXT("EvaluateStructTip", "Reference another StructChooserTable asset, evaluated at runtime if this row is selected."),
			FEvaluateStructChooser::StaticStruct());
		AddEntry(
			LOCTEXT("NestedStruct", "Nested Struct Chooser"),
			LOCTEXT("NestedStructTip", "Reference another StructChooserTable embedded in this asset, evaluated at runtime if this row is selected."),
			FNestedStructChooser::StaticStruct());
	}
}

void AddStructResultRow(UStructChooserTable* Table, const UScriptStruct* ResultType)
{
	if (!Table || !ResultType)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddStructRow", "Add Struct Chooser Row"));
	Table->Modify(true);

	FInstancedStruct& NewResult = Table->ResultsStructs.AddDefaulted_GetRef();
	InitializeStructValueIfNeeded(NewResult, Table, ResultType);
	AlignDisabledRows(Table);
	Table->Compile(true);
}

void AddStructFallbackResult(UStructChooserTable* Table, const UScriptStruct* ResultType)
{
	if (!Table || !ResultType)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddStructFallback", "Add Struct Chooser Fallback"));
	Table->Modify(true);
	InitializeStructValueIfNeeded(Table->FallbackResult, Table, ResultType);
	Table->Compile(true);
}

TSharedRef<SWidget> MakeStructChooserCreateRowMenu(
	TWeakObjectPtr<UStructChooserTable> WeakRootTable,
	TWeakPtr<UE::ChooserEditor::IChooserTableViewModel> WeakViewModel)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	TWeakObjectPtr<UStructChooserTable> WeakCurrentTable = WeakRootTable;
	if (TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> VM = WeakViewModel.Pin())
	{
		if (UStructChooserTable* Current = Cast<UStructChooserTable>(VM->GetChooser()))
		{
			WeakCurrentTable = Current;
		}
	}

	UStructChooserTable* TableForFallbackCheck = WeakCurrentTable.Get();
	if (TableForFallbackCheck && !TableForFallbackCheck->FallbackResult.IsValid())
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("AddFallback", "Add Fallback Result"),
			LOCTEXT("AddFallbackTip", "Add a Fallback row used when no other rows pass all filter columns"),
			FNewMenuDelegate::CreateLambda([WeakCurrentTable, WeakViewModel](FMenuBuilder& SubMenu)
			{
				SubMenu.BeginSection(TEXT("StructChooser"), LOCTEXT("StructChooserSection", "StructChooser"));
				AppendStructResultTypeEntries(SubMenu, WeakCurrentTable, WeakViewModel, /*bFallback*/ true);
				SubMenu.EndSection();
			}));
	}

	MenuBuilder.BeginSection(TEXT("StructChooser"), LOCTEXT("StructChooserSection", "StructChooser"));
	AppendStructResultTypeEntries(MenuBuilder, WeakCurrentTable, WeakViewModel, /*bFallback*/ false);
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void CacheStructChooserViewModel(
	UStructChooserTable* RootTable,
	TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> ViewModel)
{
	if (!RootTable || !ViewModel.IsValid())
	{
		return;
	}
	UStructChooserTable* Root = Cast<UStructChooserTable>(RootTable->GetRootChooser());
	if (!Root)
	{
		Root = RootTable;
	}
	GViewModelByRoot.Add(Root, ViewModel);
}

TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> FindCachedStructChooserViewModel(UStructChooserTable* AnyTable)
{
	if (!AnyTable)
	{
		return nullptr;
	}
	UStructChooserTable* Root = Cast<UStructChooserTable>(AnyTable->GetRootChooser());
	if (!Root)
	{
		Root = AnyTable;
	}
	if (TWeakPtr<UE::ChooserEditor::IChooserTableViewModel>* Found = GViewModelByRoot.Find(Root))
	{
		return Found->Pin();
	}
	return nullptr;
}

void ClearCachedStructChooserViewModels()
{
	GViewModelByRoot.Empty();
}
}

#undef LOCTEXT_NAMESPACE
