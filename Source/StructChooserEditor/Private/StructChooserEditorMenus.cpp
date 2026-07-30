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

			auto AddEntry = [&InSection, WeakTable, WeakViewModel](const FName Name, const FText& Label, const FText& Tooltip, UScriptStruct* Type)
			{
				InSection.AddMenuEntry(
					Name,
					Label,
					Tooltip,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([WeakTable, WeakViewModel, Type]()
					{
						if (UStructChooserTable* Table = WeakTable.Get())
						{
							AddStructResultRow(Table, Type);
							if (TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> VM = WeakViewModel.Pin())
							{
								VM->RefreshAll();
							}
						}
					})));
			};

			AddEntry(
				TEXT("AddStructValue"),
				LOCTEXT("AddStructValue", "Add Struct Row"),
				LOCTEXT("AddStructValueTip", "Add a Struct result row (same as Add Row → StructChooser → Struct)"),
				FStructValueChooser::StaticStruct());
			AddEntry(
				TEXT("AddEvaluateStruct"),
				LOCTEXT("AddEvaluateStruct", "Add Evaluate Struct Chooser Row"),
				LOCTEXT("AddEvaluateStructTip", "Add an Evaluate Struct Chooser result row"),
				FEvaluateStructChooser::StaticStruct());
			AddEntry(
				TEXT("AddNestedStruct"),
				LOCTEXT("AddNestedStruct", "Add Nested Struct Chooser Row"),
				LOCTEXT("AddNestedStructTip", "Add a Nested Struct Chooser result row"),
				FNestedStructChooser::StaticStruct());
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
