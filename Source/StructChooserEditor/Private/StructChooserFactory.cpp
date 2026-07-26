#include "StructChooserFactory.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"
#include "StructViewerModule.h"
#include "StructViewerFilter.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SWindow.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserFactory)

#define LOCTEXT_NAMESPACE "StructChooserFactory"

namespace
{
class FStructChooserStructFilter : public IStructViewerFilter
{
public:
	virtual bool IsStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const UScriptStruct* InStruct, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		return InStruct != nullptr && !InStruct->HasMetaData(TEXT("Hidden"));
	}

	virtual bool IsUnloadedStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const FSoftObjectPath& InStructPath, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		return true;
	}
};
}

UStructChooserTableFactory::UStructChooserTableFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UStructChooserTable::StaticClass();
}

bool UStructChooserTableFactory::ConfigureProperties()
{
	class FStructChooserCreateDialog : public TSharedFromThis<FStructChooserCreateDialog>
	{
	public:
		UScriptStruct* ResultStruct = nullptr;
		bool bConfirmed = false;
		TSharedPtr<SWindow> Window;

		void Open(UScriptStruct*& OutStruct)
		{
			FStructViewerModule& StructViewerModule = FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer");
			FStructViewerInitializationOptions Options;
			Options.Mode = EStructViewerMode::StructPicker;
			Options.StructFilter = MakeShared<FStructChooserStructFilter>();

			Window = SNew(SWindow)
				.Title(LOCTEXT("PickStructTitle", "Pick Result Struct Type"))
				.ClientSize(FVector2D(400, 500))
				.SupportsMinimize(false)
				.SupportsMaximize(false)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						StructViewerModule.CreateStructViewer(Options, FOnStructPicked::CreateRaw(this, &FStructChooserCreateDialog::OnPicked))
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(8)
					[
						SNew(SUniformGridPanel).SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
							.Text(LOCTEXT("Cancel", "Cancel"))
							.HAlign(HAlign_Center)
							.OnClicked_Lambda([this]()
							{
								bConfirmed = false;
								Window->RequestDestroyWindow();
								return FReply::Handled();
							})
						]
					]
				];

			GEditor->EditorAddModalWindow(Window.ToSharedRef());
			if (bConfirmed)
			{
				OutStruct = ResultStruct;
			}
		}

		void OnPicked(const UScriptStruct* Chosen)
		{
			ResultStruct = const_cast<UScriptStruct*>(Chosen);
			bConfirmed = ResultStruct != nullptr;
			if (Window.IsValid())
			{
				Window->RequestDestroyWindow();
			}
		}
	};

	UScriptStruct* Chosen = nullptr;
	TSharedRef<FStructChooserCreateDialog> Dialog = MakeShared<FStructChooserCreateDialog>();
	Dialog->Open(Chosen);
	if (!Chosen)
	{
		return false;
	}

	OutputStructType = Chosen;
	return true;
}

UObject* UStructChooserTableFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UStructChooserTable* NewChooser = NewObject<UStructChooserTable>(InParent, Class, Name, Flags);
	NewChooser->ApplyStructChooserDefaults();
	NewChooser->OutputStructType = OutputStructType;
	NewChooser->Version = UChooserTable::CurrentVersion;

#if WITH_EDITORONLY_DATA
	FInstancedStruct& FirstRow = NewChooser->ResultsStructs.AddDefaulted_GetRef();
	FirstRow.InitializeAs(FStructValueChooser::StaticStruct());
	if (OutputStructType)
	{
		FirstRow.GetMutable<FStructValueChooser>().Value.InitializeAs(OutputStructType);
	}
#endif

	return NewChooser;
}

#undef LOCTEXT_NAMESPACE
