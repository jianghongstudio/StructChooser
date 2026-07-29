#include "StructChooserTableToolbar.h"
#include "ChooserTableEditorCommands.h"
#include "IChooserTableViewModel.h"
#include "StructChooserTableEditor.h"
#include "StructChooserTableViewModel.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "StructChooserTableToolbar"

namespace UE::StructChooserEditor
{
void RegisterStructChooserTableToolbar()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	UToolMenu* ToolBar = nullptr;
	if (ToolMenus->IsMenuRegistered(FStructChooserTableEditor::StructChooserTableToolbarName))
	{
		ToolBar = ToolMenus->ExtendMenu(FStructChooserTableEditor::StructChooserTableToolbarName);
	}
	else
	{
		ToolBar = ToolMenus->RegisterMenu(
			FStructChooserTableEditor::StructChooserTableToolbarName,
			NAME_None,
			EMultiBoxType::SlimHorizontalToolBar);
	}

	const FChooserTableEditorCommands& Commands = FChooserTableEditorCommands::Get();
	FToolMenuSection& Section = ToolBar->AddSection(TEXT("StructChooser"), TAttribute<FText>());
	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		Commands.EditChooserSettings,
		TAttribute<FText>(),
		TAttribute<FText>(),
		FSlateIcon(TEXT("EditorStyle"), TEXT("FullBlueprintEditor.EditGlobalOptions"))));

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(Commands.AutoPopulateAll));

	Section.AddDynamicEntry(TEXT("DebuggingCommands"), FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		UChooserEditorToolMenuContext* Context = InSection.FindContext<UChooserEditorToolMenuContext>();
		if (!Context)
		{
			return;
		}

		TSharedPtr<IChooserTableViewModel> ViewModel = Context->ViewModel.Pin();
		if (!ViewModel.IsValid())
		{
			return;
		}

		InSection.AddEntry(FToolMenuEntry::InitComboButton(
			TEXT("SelectDebugTarget"),
			FToolUIActionChoice(),
			FNewToolMenuDelegate::CreateLambda([ViewModel](UToolMenu* InToolMenu)
			{
				if (FStructChooserTableViewModel* TypedViewModel = static_cast<FStructChooserTableViewModel*>(ViewModel.Get()))
				{
					TypedViewModel->MakeDebugTargetMenu(InToolMenu);
				}
			}),
			TAttribute<FText>::CreateLambda([ViewModel]()
			{
				UChooserTable* Chooser = ViewModel->GetRootChooser();
				if (Chooser->HasDebugTarget())
				{
					return FText::FromString(Chooser->GetDebugTargetName());
				}
				return Chooser->GetEnableDebugTesting()
					? LOCTEXT("ManualTesting", "Manual Testing")
					: LOCTEXT("DebugTarget", "Debug Target");
			}),
			LOCTEXT("DebugTargetTooltip", "Select an object that has recently been the context object for this chooser to visualize the selection results")));
	}));
}
}

#undef LOCTEXT_NAMESPACE
