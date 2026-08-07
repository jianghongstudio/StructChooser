#include "StructChooserEditorMenus.h"

#include "IChooserTableViewModel.h"
#include "StructChooserRowActions.h"
#include "StructChooserTable.h"
#include "StructChooserTableEditor.h"
#include "StructChooserTypes.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "StructChooserEditorMenus"

namespace UE::StructChooserEditor
{
namespace
{
	static const FName ContextMenuName(TEXT("StructChooserEditorContextMenu"));

	static void CacheViewModelFromSection(FToolMenuSection& InSection)
	{
		const UStructChooserEditorToolMenuContext* Context = InSection.FindContext<UStructChooserEditorToolMenuContext>();
		if (!Context || !Context->ViewModel.IsValid())
		{
			return;
		}
		TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> ViewModel = Context->ViewModel.Pin();
		if (!ViewModel.IsValid())
		{
			return;
		}
		if (UStructChooserTable* Table = Cast<UStructChooserTable>(ViewModel->GetRootChooser()))
		{
			CacheStructChooserViewModel(Table, ViewModel);
		}
	}
}

void RegisterStructChooserEditorMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	if (UToolMenu* Toolbar = ToolMenus->ExtendMenu(FStructChooserTableEditor::StructChooserTableToolbarName))
	{
		FToolMenuSection& ToolbarSection = Toolbar->AddSection(TEXT("StructChooserViewModelCache"));
		ToolbarSection.AddDynamicEntry(
			TEXT("CacheViewModel"),
			FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				CacheViewModelFromSection(InSection);
			}));
	}

	UToolMenu* Menu = ToolMenus->ExtendMenu(ContextMenuName);
	if (!Menu)
	{
		Menu = ToolMenus->RegisterMenu(ContextMenuName, NAME_None, EMultiBoxType::Menu);
	}

	FToolMenuSection& Section = Menu->AddSection(
		TEXT("StructChooserRows"),
		LOCTEXT("StructChooserRowsSection", "StructChooser"));

	Section.AddDynamicEntry(
		TEXT("StructChooserAddRows"),
		FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			CacheViewModelFromSection(InSection);

			const UStructChooserEditorToolMenuContext* Context = InSection.FindContext<UStructChooserEditorToolMenuContext>();
			if (!Context || !Context->ViewModel.IsValid())
			{
				return;
			}

			TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> ViewModel = Context->ViewModel.Pin();
			if (!ViewModel.IsValid())
			{
				return;
			}

			TWeakObjectPtr<UStructChooserTable> WeakTable = Cast<UStructChooserTable>(ViewModel->GetChooser());
			if (!WeakTable.IsValid())
			{
				return;
			}

			TWeakPtr<UE::ChooserEditor::IChooserTableViewModel> WeakViewModel = ViewModel;

			auto Refresh = [WeakViewModel]()
			{
				if (TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> VM = WeakViewModel.Pin())
				{
					VM->RefreshAll();
				}
			};

			auto AddEntry = [&InSection, WeakTable, Refresh](const FName Name, const FText& Label, const FText& Tooltip, UScriptStruct* Type, bool bFallback)
			{
				InSection.AddMenuEntry(
					Name,
					Label,
					Tooltip,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([WeakTable, Refresh, Type, bFallback]()
					{
						if (UStructChooserTable* Table = WeakTable.Get())
						{
							if (bFallback)
							{
								AddStructFallbackResult(Table, Type);
							}
							else
							{
								AddStructResultRow(Table, Type);
							}
							Refresh();
						}
					})));
			};

			if (UStructChooserTable* Table = WeakTable.Get(); Table && !Table->FallbackResult.IsValid())
			{
				InSection.AddSubMenu(
					TEXT("AddFallbackResult"),
					LOCTEXT("AddFallback", "Add Fallback Result"),
					LOCTEXT("AddFallbackTip", "Add a Fallback row used when no other rows pass all filter columns"),
					FNewToolMenuDelegate::CreateLambda([WeakTable, Refresh](UToolMenu* SubMenu)
					{
						FToolMenuSection& FallbackSection = SubMenu->AddSection(
							TEXT("StructChooserFallback"),
							LOCTEXT("StructChooserSection", "StructChooser"));
						auto AddFallbackEntry = [&FallbackSection, WeakTable, Refresh](const FName Name, const FText& Label, const FText& Tooltip, UScriptStruct* Type)
						{
							FallbackSection.AddMenuEntry(
								Name,
								Label,
								Tooltip,
								FSlateIcon(),
								FUIAction(FExecuteAction::CreateLambda([WeakTable, Refresh, Type]()
								{
									if (UStructChooserTable* Table = WeakTable.Get())
									{
										AddStructFallbackResult(Table, Type);
										Refresh();
									}
								})));
						};
						AddFallbackEntry(TEXT("FallbackStruct"), LOCTEXT("Struct", "Struct"), LOCTEXT("StructTip", "Concrete struct fallback"), FStructValueChooser::StaticStruct());
						AddFallbackEntry(TEXT("FallbackEvaluate"), LOCTEXT("EvaluateStruct", "Evaluate Struct Chooser"), LOCTEXT("EvaluateStructTip", "Evaluate another StructChooserTable (asset or embedded)"), FEvaluateStructChooser::StaticStruct());
						AddFallbackEntry(TEXT("FallbackNested"), LOCTEXT("NestedStruct", "Nested Struct Chooser"), LOCTEXT("NestedStructTip", "Evaluate an embedded StructChooserTable"), FNestedStructChooser::StaticStruct());
					}));
			}

			AddEntry(
				TEXT("AddStructValue"),
				LOCTEXT("AddStructValue", "Add Struct Row"),
				LOCTEXT("AddStructValueTip", "Add a Struct result row (same as Add Row → StructChooser → Struct)"),
				FStructValueChooser::StaticStruct(),
				false);
			AddEntry(
				TEXT("AddEvaluateStruct"),
				LOCTEXT("AddEvaluateStruct", "Add Evaluate Struct Chooser Row"),
				LOCTEXT("AddEvaluateStructTip", "Add an Evaluate Struct Chooser result row"),
				FEvaluateStructChooser::StaticStruct(),
				false);
			AddEntry(
				TEXT("AddNestedStruct"),
				LOCTEXT("AddNestedStruct", "Add Nested Struct Chooser Row"),
				LOCTEXT("AddNestedStructTip", "Add a Nested Struct Chooser result row"),
				FNestedStructChooser::StaticStruct(),
				false);
		}));
}

void UnregisterStructChooserEditorMenus()
{
	ClearCachedStructChooserViewModels();

	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		if (UToolMenu* Menu = ToolMenus->FindMenu(ContextMenuName))
		{
			Menu->RemoveSection(TEXT("StructChooserRows"));
		}
		if (UToolMenu* Toolbar = ToolMenus->FindMenu(FStructChooserTableEditor::StructChooserTableToolbarName))
		{
			Toolbar->RemoveSection(TEXT("StructChooserViewModelCache"));
		}
	}
}
}

#undef LOCTEXT_NAMESPACE
