#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "StructChooserDetails.h"
#include "StructChooserEditorWidgets.h"
#include "StructChooserRowDetails.h"
#include "StructChooserTable.h"

class FStructChooserEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// Ensure engine ChooserEditor has registered its ChooserRowDetails layout first;
		// our registration replaces it (TMap::Add).
		FModuleManager::LoadModuleChecked<IModuleInterface>("ChooserEditor");

		UE::StructChooserEditor::RegisterStructChooserWidgets();

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(
			UStructChooserTable::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FStructChooserDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(
			TEXT("ChooserRowDetails"),
			FOnGetDetailCustomizationInstance::CreateStatic(&FStructChooserRowDetails::MakeInstance));
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyModule.UnregisterCustomClassLayout(UStructChooserTable::StaticClass()->GetFName());
			// Restore engine layout if ChooserEditor is still loaded
			PropertyModule.UnregisterCustomClassLayout(TEXT("ChooserRowDetails"));
		}
	}
};

IMPLEMENT_MODULE(FStructChooserEditorModule, StructChooserEditor)
