#include "Modules/ModuleManager.h"
#include "ObjectChooserWidgetFactories.h"
#include "StructChooserEditorWidgets.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"

class FStructChooserEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE::StructChooserEditor::RegisterStructChooserWidgets();

		UE::ChooserEditor::FObjectChooserWidgetFactories::RegisterResultTypeFilter(
			[](const UChooserTable* Chooser, const UScriptStruct* ResultType) -> bool
			{
				const bool bIsStructChooserResult = ResultType->IsChildOf(FStructChooserBase::StaticStruct());
				const bool bIsStructChooserTable = Chooser && Chooser->IsA<UStructChooserTable>();

				// StructChooser tables: only StructChooser result row types
				if (bIsStructChooserTable)
				{
					return bIsStructChooserResult;
				}

				// Normal Chooser tables: hide StructChooser-only result types
				return !bIsStructChooserResult;
			});
	}
};

IMPLEMENT_MODULE(FStructChooserEditorModule, StructChooserEditor)
