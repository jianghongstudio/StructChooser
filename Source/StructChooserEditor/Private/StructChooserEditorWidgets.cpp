#include "StructChooserEditorWidgets.h"
#include "StructChooserTypes.h"
#include "StructChooserTable.h"
#include "StructChooserStructFilter.h"
#include "Chooser.h"
#include "ChooserEditorStyle.h"
#include "ScopedTransaction.h"
#include "Editor.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/AssetRegistryTagsContext.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "PropertyCustomizationHelpers.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Styling/AppStyle.h"
#include "DetailLayoutBuilder.h"
#include "Modules/ModuleManager.h"
#include "StructViewerModule.h"

#define LOCTEXT_NAMESPACE "StructChooserEditorWidgets"

namespace UE::StructChooserEditor
{
using namespace UE::ChooserEditor;

static void EnsureStructValueType(FStructValueChooser* StructChooser, UObject* TransactionObject)
{
	if (!StructChooser)
	{
		return;
	}

	UScriptStruct* DesiredType = nullptr;
	if (UStructChooserTable* Table = Cast<UStructChooserTable>(TransactionObject))
	{
		if (UStructChooserTable* Root = Cast<UStructChooserTable>(Table->GetRootChooser()))
		{
			DesiredType = Root->OutputStructType;
		}
		if (!DesiredType)
		{
			DesiredType = Table->OutputStructType;
		}
	}

	if (DesiredType && (!StructChooser->Value.IsValid() || StructChooser->Value.GetScriptStruct() != DesiredType))
	{
		StructChooser->Value.InitializeAs(DesiredType);
	}
}

static TSharedRef<SWidget> CreateStructValueChooserWidget(bool bReadOnly, UObject* TransactionObject, void* Value, UClass* ResultBaseClass, FChooserWidgetValueChanged ValueChanged)
{
	FStructValueChooser* StructChooser = static_cast<FStructValueChooser*>(Value);
	EnsureStructValueType(StructChooser, TransactionObject);

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.Padding(2.f, 1.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.IsReadOnly(bReadOnly)
				.HintText(LOCTEXT("StructNameHint", "Enter name..."))
				.ToolTipText(LOCTEXT("StructNameTooltip", "Display name for this result row. Edit the struct fields in the Details panel."))
				.Text_Lambda([StructChooser]()
				{
					return FText::FromString(StructChooser->Name);
				})
				.OnTextCommitted_Lambda([TransactionObject, StructChooser, ValueChanged](const FText& NewText, ETextCommit::Type)
				{
					const FString NewName = NewText.ToString();
					if (StructChooser->Name == NewName)
					{
						return;
					}

					const FScopedTransaction Transaction(LOCTEXT("EditStructResultName", "Edit Struct Result Name"));
					if (TransactionObject)
					{
						TransactionObject->Modify(true);
					}
					StructChooser->Name = NewName;
					ValueChanged.ExecuteIfBound();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(6.f, 0.f, 2.f, 0.f)
			[
				SNew(STextBlock)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Text_Lambda([StructChooser]()
				{
					if (StructChooser->Value.IsValid() && StructChooser->Value.GetScriptStruct())
					{
						return StructChooser->Value.GetScriptStruct()->GetDisplayNameText();
					}
					return LOCTEXT("StructTypeUnknown", "No type");
				})
				.ToolTipText(LOCTEXT("StructTypeTooltip", "Result struct type. Edit field values in the Details panel."))
			]
		];
}

static TSharedRef<SWidget> CreateEvaluateStructChooserWidget(bool bReadOnly, UObject* TransactionObject, void* Value, UClass* ResultBaseClass, FChooserWidgetValueChanged ValueChanged)
{
	FEvaluateStructChooser* EvaluateChooser = static_cast<FEvaluateStructChooser*>(Value);

	UScriptStruct* ExpectedStructType = nullptr;
	if (UStructChooserTable* Owner = Cast<UStructChooserTable>(TransactionObject))
	{
		UStructChooserTable* Root = Cast<UStructChooserTable>(Owner->GetRootChooser());
		ExpectedStructType = Root ? Root->OutputStructType.Get() : Owner->OutputStructType.Get();
	}

	return SNew(SObjectPropertyEntryBox)
		.IsEnabled(!bReadOnly)
		.AllowedClass(UStructChooserTable::StaticClass())
		.ObjectPath_Lambda([EvaluateChooser]()
		{
			return EvaluateChooser->Chooser ? EvaluateChooser->Chooser.GetPath() : FString();
		})
		.OnShouldFilterAsset_Lambda([ExpectedStructType](const FAssetData& InAssetData)
		{
			if (!InAssetData.IsInstanceOf(UStructChooserTable::StaticClass()))
			{
				return true;
			}
			if (!ExpectedStructType)
			{
				return false;
			}
			if (UStructChooserTable* Other = Cast<UStructChooserTable>(InAssetData.GetAsset()))
			{
				UStructChooserTable* Root = Cast<UStructChooserTable>(Other->GetRootChooser());
				const UScriptStruct* OtherType = Root ? Root->OutputStructType.Get() : Other->OutputStructType.Get();
				return OtherType != ExpectedStructType;
			}
			return true;
		})
		.OnObjectChanged_Lambda([TransactionObject, EvaluateChooser, ValueChanged](const FAssetData& AssetData)
		{
			const FScopedTransaction Transaction(LOCTEXT("EditEvaluateStructChooser", "Edit Evaluate Struct Chooser"));
			TransactionObject->Modify(true);
			EvaluateChooser->Chooser = Cast<UStructChooserTable>(AssetData.GetAsset());
			ValueChanged.ExecuteIfBound();
		});
}

static TSharedRef<SWidget> CreateNestedStructChooserWidget(bool bReadOnly, UObject* TransactionObject, void* Value, UClass* ResultBaseClass, FChooserWidgetValueChanged ValueChanged, IChooserTableWidgetInterface* ChooserWidgetInterface)
{
	FNestedStructChooser* NestedChooser = static_cast<FNestedStructChooser*>(Value);

	TSharedRef<SComboButton> Button = SNew(SComboButton)
		.IsEnabled(!bReadOnly)
		.ContentPadding(0)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SImage).Image(FChooserEditorStyle::Get().GetBrush("ChooserEditor.NestedChooserIcon"))
			]
			+ SHorizontalBox::Slot().FillWidth(1.0).Padding(2)
			[
				SNew(STextBlock)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.Text_Lambda([NestedChooser]()
				{
					return NestedChooser->Chooser
						? FText::FromString(NestedChooser->Chooser->GetName())
						: LOCTEXT("None", "None");
				})
			]
		];

	Button->SetOnGetMenuContent(FOnGetContent::CreateLambda([Button, NestedChooser, TransactionObject]()
	{
		FMenuBuilder MenuBuilder(true, nullptr);
		MenuBuilder.BeginSection(NAME_None, LOCTEXT("NestedStructChooser", "Nested Struct Chooser"));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("None", "None"),
			LOCTEXT("ClearNested", "Clear nested reference"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([Button, NestedChooser, TransactionObject]()
			{
				const FScopedTransaction Transaction(LOCTEXT("ClearNestedStructChooser", "Clear Nested Struct Chooser"));
				TransactionObject->Modify(true);
				Button->SetIsOpen(false);
				NestedChooser->Chooser = nullptr;
			})));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("NewNestedStructChooser", "New Nested Struct Chooser"),
			LOCTEXT("NewNestedStructChooserTip", "Create an embedded StructChooserTable and reference it"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([Button, NestedChooser, TransactionObject]()
			{
				Button->SetIsOpen(false);

				TSharedRef<STextEntryPopup> TextEntry =
					SNew(STextEntryPopup)
					.Label(LOCTEXT("NewNestedName", "New Chooser Name"))
					.OnTextCommitted_Lambda([NestedChooser, TransactionObject](FText InText, ETextCommit::Type InCommitType)
					{
						if (InCommitType != ETextCommit::OnEnter)
						{
							return;
						}

						FSlateApplication::Get().DismissAllMenus();
						FName NewChooserName = FName(InText.ToString());
						if (NewChooserName == NAME_None || FindObject<UObject>(TransactionObject, *NewChooserName.ToString()))
						{
							NewChooserName = MakeUniqueObjectName(TransactionObject, UStructChooserTable::StaticClass(), NewChooserName);
						}

						UStructChooserTable* NewChooser = NewObject<UStructChooserTable>(TransactionObject, UStructChooserTable::StaticClass(), NewChooserName, RF_Transactional);
						NewChooser->ApplyStructChooserDefaults();

						if (UStructChooserTable* Parent = Cast<UStructChooserTable>(TransactionObject))
						{
							UStructChooserTable* Root = Cast<UStructChooserTable>(Parent->GetRootChooser());
							if (Root && Root->OutputStructType)
							{
								NewChooser->OutputStructType = Root->OutputStructType;
							}
							else if (Parent->OutputStructType)
							{
								NewChooser->OutputStructType = Parent->OutputStructType;
							}
						}

						const FScopedTransaction Transaction(LOCTEXT("AssignNestedStructChooser", "Assign Nested Struct Chooser"));
						TransactionObject->Modify();
						NestedChooser->Chooser = NewChooser;

						if (UChooserTable* ParentChooser = Cast<UChooserTable>(TransactionObject))
						{
							UChooserTable* RootChooser = ParentChooser->GetRootChooser();
							if (RootChooser)
							{
								NewChooser->RootChooser = RootChooser;
								RootChooser->Modify();
								RootChooser->AddNestedObject(NewChooser);
								if (IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
								{
									AssetRegistry->AssetUpdateTags(RootChooser, EAssetRegistryTagsCaller::Fast);
								}
							}
						}
					});

				FSlateApplication& SlateApp = FSlateApplication::Get();
				SlateApp.PushMenu(
					SlateApp.GetInteractiveTopLevelWindows()[0],
					FWidgetPath(),
					TextEntry,
					SlateApp.GetCursorPos(),
					FPopupTransitionEffect::TypeInPopup);
			})));

		MenuBuilder.AddSubMenu(
			LOCTEXT("SelectExisting", "Select Existing"),
			LOCTEXT("SelectExistingTip", "Select an existing embedded StructChooserTable from this asset"),
			FNewMenuDelegate::CreateLambda([Button, NestedChooser, TransactionObject](FMenuBuilder& SubMenuBuilder)
			{
				SubMenuBuilder.BeginSection("Existing", LOCTEXT("Existing", "Existing"));
				if (UChooserTable* OuterChooser = Cast<UChooserTable>(TransactionObject))
				{
					UChooserTable* RootTable = OuterChooser->GetRootChooser();
					for (UObject* Object : RootTable->NestedObjects)
					{
						if (UStructChooserTable* Chooser = Cast<UStructChooserTable>(Object))
						{
							if (Chooser != RootTable)
							{
								SubMenuBuilder.AddMenuEntry(
									FText::FromString(Chooser->GetName()),
									LOCTEXT("AddExistingTip", "Reference this embedded StructChooserTable"),
									FSlateIcon(),
									FUIAction(FExecuteAction::CreateLambda([Chooser, Button, NestedChooser, TransactionObject]()
									{
										const FScopedTransaction Transaction(LOCTEXT("SetNestedStructChooser", "Set Nested Struct Chooser"));
										TransactionObject->Modify(true);
										Button->SetIsOpen(false);
										NestedChooser->Chooser = Chooser;
									})));
							}
						}
					}
				}
				SubMenuBuilder.EndSection();
			}));

		MenuBuilder.EndSection();
		return MenuBuilder.MakeWidget();
	}));

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0)
		[
			Button
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Edit", "Edit"))
			.OnClicked_Lambda([NestedChooser, TransactionObject, ChooserWidgetInterface]()
			{
				if (NestedChooser->Chooser)
				{
					if (ChooserWidgetInterface)
					{
						ChooserWidgetInterface->OpenObject(NestedChooser->Chooser);
					}
					else if (UObject* RootChooser = TransactionObject->GetPackage()->FindAssetInPackage())
					{
						if (IAssetEditorInstance* Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(RootChooser, false))
						{
							Editor->FocusWindow(NestedChooser->Chooser);
						}
					}
				}
				return FReply::Handled();
			})
		];
}

void RegisterStructChooserWidgets()
{
	FObjectChooserWidgetFactories::RegisterWidgetCreator(FStructValueChooser::StaticStruct(), CreateStructValueChooserWidget);
	FObjectChooserWidgetFactories::RegisterWidgetCreator(FEvaluateStructChooser::StaticStruct(), CreateEvaluateStructChooserWidget);
	FObjectChooserWidgetFactories::RegisterWidgetCreator(FNestedStructChooser::StaticStruct(), CreateNestedStructChooserWidget);
}

static void InitializeStructResultOfType(UChooserTable* TransactionObject, const UScriptStruct* ChosenStruct, FInstancedStruct& OutResult)
{
	if (!ChosenStruct)
	{
		OutResult.Reset();
		return;
	}

	if (ChosenStruct == FStructValueChooser::StaticStruct())
	{
		OutResult.InitializeAs(FStructValueChooser::StaticStruct());
		UScriptStruct* OutputType = nullptr;
		if (UStructChooserTable* Table = Cast<UStructChooserTable>(TransactionObject))
		{
			if (UStructChooserTable* Root = Cast<UStructChooserTable>(Table->GetRootChooser()))
			{
				OutputType = Root->OutputStructType;
			}
			if (!OutputType)
			{
				OutputType = Table->OutputStructType;
			}
		}
		if (OutputType)
		{
			OutResult.GetMutable<FStructValueChooser>().Value.InitializeAs(OutputType);
		}
		return;
	}

	OutResult.InitializeAs(ChosenStruct);
}

TSharedPtr<SWidget> CreateStructChooserResultCellWidget(
	bool bReadOnly,
	UChooserTable* TransactionObject,
	FInstancedStruct* ResultData,
	FChooserWidgetValueChanged ValueChanged,
	IChooserTableWidgetInterface* ChooserWidgetInterface,
	TSharedPtr<SBorder>* InnerWidget,
	FText NullValueDisplayText)
{
	if (!ResultData)
	{
		return nullptr;
	}

	UClass* ResultBaseClass = nullptr;
	if (TransactionObject)
	{
		if (UChooserTable* Root = TransactionObject->GetRootChooser())
		{
			ResultBaseClass = Root->OutputObjectType;
		}
	}

	TSharedPtr<SWidget> LeftWidget = FObjectChooserWidgetFactories::CreateWidget(
		bReadOnly,
		TransactionObject,
		ResultData->GetMutableMemory(),
		ResultData->GetScriptStruct(),
		ResultBaseClass,
		ValueChanged,
		ChooserWidgetInterface);

	if (bReadOnly)
	{
		return LeftWidget;
	}

	if (!LeftWidget.IsValid())
	{
		LeftWidget = SNew(STextBlock)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Margin(2)
			.Text(NullValueDisplayText.IsEmpty() ? LOCTEXT("SelectDataType", "Select Data Type...") : NullValueDisplayText);
	}

	TSharedPtr<SComboButton> Button = SNew(SComboButton)
		.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButton");

	TWeakObjectPtr<UChooserTable> WeakTable = TransactionObject;
	TSharedPtr<SBorder> Border = (InnerWidget && InnerWidget->IsValid()) ? *InnerWidget : SNew(SBorder);
	if (InnerWidget)
	{
		*InnerWidget = Border;
	}

	Button->SetOnGetMenuContent(FOnGetContent::CreateLambda([WeakTable, ResultData, Button, Border, ValueChanged, ChooserWidgetInterface, NullValueDisplayText]()
	{
		FStructViewerInitializationOptions Options;
		Options.StructFilter = MakeShared<FStructChooserResultFilter>();
		Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
		Options.bShowNoneOption = false;

		FStructViewerModule& StructViewer = FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer");
		return StructViewer.CreateStructViewer(Options, FOnStructPicked::CreateLambda(
			[WeakTable, ResultData, Button, Border, ValueChanged, ChooserWidgetInterface, NullValueDisplayText](const UScriptStruct* ChosenStruct)
			{
				Button->SetIsOpen(false);
				UChooserTable* Table = WeakTable.Get();
				if (!Table || !ResultData || !ChosenStruct)
				{
					return;
				}

				const FScopedTransaction Transaction(LOCTEXT("ChangeStructRowResultType", "Change Struct Chooser Result Type"));
				Table->Modify(true);
				InitializeStructResultOfType(Table, ChosenStruct, *ResultData);

				TSharedPtr<SWidget> NewContent = FObjectChooserWidgetFactories::CreateWidget(
					false,
					Table,
					ResultData->GetMutableMemory(),
					ResultData->GetScriptStruct(),
					Table->GetRootChooser() ? Table->GetRootChooser()->OutputObjectType.Get() : nullptr,
					ValueChanged,
					ChooserWidgetInterface);

				if (!NewContent.IsValid())
				{
					NewContent = SNew(STextBlock)
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
						.Margin(2)
						.Text(NullValueDisplayText.IsEmpty() ? LOCTEXT("SelectDataType", "Select Data Type...") : NullValueDisplayText);
				}
				Border->SetContent(NewContent.ToSharedRef());
				ValueChanged.ExecuteIfBound();
			}));
	}));

	Border->SetContent(LeftWidget.ToSharedRef());

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(100)
		[
			Border.ToSharedRef()
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			Button.ToSharedRef()
		];
}

}

#undef LOCTEXT_NAMESPACE
