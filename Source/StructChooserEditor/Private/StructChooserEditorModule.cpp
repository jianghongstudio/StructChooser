#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ChooserTableEditorCommands.h"
#include "StructChooserDetails.h"
#include "StructChooserEditorMenus.h"
#include "StructChooserEditorWidgets.h"
#include "StructChooserTable.h"
#include "StructChooserTableEditor.h"
#include "StructChooserTableToolbar.h"

class FStructChooserEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FModuleManager::LoadModuleChecked<IModuleInterface>("ChooserEditor");

		// Header-only TCommands are per-DLL: ChooserEditor::Register() does not populate
		// this module's Instance. Register a local copy before any ::Get() (toolbar/menus).
		FChooserTableEditorCommands::Register();

		UE::StructChooserEditor::RegisterStructChooserWidgets();
		UE::StructChooserEditor::RegisterStructChooserEditorMenus();
		UE::StructChooserEditor::RegisterStructChooserTableToolbar();
		UE::StructChooserEditor::FStructChooserTableEditor::RegisterWidgets();

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(
			UStructChooserTable::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FStructChooserDetails::MakeInstance));
	}

	virtual void ShutdownModule() override
	{
		UE::StructChooserEditor::UnregisterStructChooserEditorMenus();

		if (FChooserTableEditorCommands::IsRegistered())
		{
			FChooserTableEditorCommands::Unregister();
		}

		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyModule.UnregisterCustomClassLayout(UStructChooserTable::StaticClass()->GetFName());
			PropertyModule.UnregisterCustomClassLayout(TEXT("StructChooserRowDetails"));
			PropertyModule.UnregisterCustomClassLayout(TEXT("StructChooserColumnDetails"));
		}
	}
};

IMPLEMENT_MODULE(FStructChooserEditorModule, StructChooserEditor)
