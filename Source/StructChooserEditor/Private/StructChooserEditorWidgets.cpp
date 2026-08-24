#include "StructChooserEditorWidgets.h"
#include "StructChooserTypes.h"
#include "StructChooserTable.h"
#include "StructChooserStructFilter.h"
#include "Chooser.h"
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
#include "Styling/SlateStyleRegistry.h"
#include "DetailLayoutBuilder.h"
#include "Modules/ModuleManager.h"
#include "StructViewerModule.h"

#define LOCTEXT_NAMESPACE "StructChooserEditorWidgets"

namespace UE::StructChooserEditor
{
using namespace UE::ChooserEditor;

static const FSlateBrush* GetChooserEditorBrush(const FName BrushName)
{
	if (const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(TEXT("ChooserEditorStyle")))
	{
		return Style->GetBrush(BrushName);
	}
	return nullptr;
}

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
				.Visibility_Lambda([StructChooser]()
				{
					// Named rows: show only the name. Type is a fallback when Name is empty.
					return StructChooser->Name.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed;
				})
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

static bool MatchesExpectedOutputStructType(UStructChooserTable* Other, UScriptStruct* ExpectedStructType)
{
	if (!Other)
	{
		return false;
	}
	if (!ExpectedStructType)
	{
		return true;
	}
	UStructChooserTable* Root = Cast<UStructChooserTable>(Other->GetRootChooser());
	const UScriptStruct* OtherType = Root ? Root->OutputStructType.Get() : Other->OutputStructType.Get();
	return OtherType == ExpectedStructType;
}

static bool IsChooserWithinRoot(const UChooserTable* Chooser, const UChooserTable* RootChooser)
{
	for (const UChooserTable* Current = Chooser; Current; Current = Cast<UChooserTable>(Current->GetOuter()))
	{
		if (Current == RootChooser)
		{
			return true;
		}
	}
	return false;
}

static void OpenStructChooserForEditing(UObject* TransactionObject, UChooserTable* ReferencedChooser)
{
	if (!GEditor || !TransactionObject || !ReferencedChooser)
	{
		return;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		return;
	}

	UChooserTable* OwnerChooser = Cast<UChooserTable>(TransactionObject);
	UChooserTable* OwnerRoot = OwnerChooser ? OwnerChooser->GetRootChooser() : nullptr;
	if (OwnerRoot && IsChooserWithinRoot(ReferencedChooser, OwnerRoot))
	{
		if (IAssetEditorInstance* Editor = AssetEditorSubsystem->FindEditorForAsset(OwnerRoot, false))
		{
			Editor->FocusWindow(ReferencedChooser);
			return;
		}
	}

	UChooserTable* ReferencedRoot = ReferencedChooser->GetRootChooser();
	UObject* AssetToOpen = ReferencedRoot && ReferencedRoot->IsAsset()
		? ReferencedRoot
		: ReferencedChooser->GetOutermost()->FindAssetInPackage();
	if (AssetToOpen)
	{
		AssetEditorSubsystem->OpenEditorForAsset(AssetToOpen);
	}
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

	// Asset picker covers external tables; nested subobjects are not in the asset registry,
	// so also expose Select Existing (same-asset NestedObjects) — needed for Fallback → child table.
	TSharedRef<SComboButton> NestedPickButton = SNew(SComboButton)
		.IsEnabled(!bReadOnly)
		.ContentPadding(0)
		.ToolTipText(LOCTEXT("EvaluatePickNestedTip", "Pick an embedded StructChooserTable from this asset (including Fallback → nested child)"))
		.ButtonContent()
		[
			SNew(SImage).Image(GetChooserEditorBrush(TEXT("ChooserEditor.ChooserTableIconSmall")))
		];

	NestedPickButton->SetOnGetMenuContent(FOnGetContent::CreateLambda(
		[NestedPickButton, EvaluateChooser, TransactionObject, ExpectedStructType, ValueChanged]()
		{
			FMenuBuilder MenuBuilder(true, nullptr);
			MenuBuilder.BeginSection(NAME_None, LOCTEXT("EvaluateNestedSection", "Embedded Struct Chooser"));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("None", "None"),
				LOCTEXT("ClearEvaluate", "Clear Evaluate reference"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([NestedPickButton, EvaluateChooser, TransactionObject, ValueChanged]()
				{
					const FScopedTransaction Transaction(LOCTEXT("ClearEvaluateStructChooser", "Clear Evaluate Struct Chooser"));
					TransactionObject->Modify(true);
					NestedPickButton->SetIsOpen(false);
					EvaluateChooser->Chooser = nullptr;
					ValueChanged.ExecuteIfBound();
				})));

			MenuBuilder.AddSubMenu(
				LOCTEXT("SelectExisting", "Select Existing"),
				LOCTEXT("SelectExistingEvaluateTip", "Select an existing embedded StructChooserTable from this asset"),
				FNewMenuDelegate::CreateLambda([NestedPickButton, EvaluateChooser, TransactionObject, ExpectedStructType, ValueChanged](FMenuBuilder& SubMenuBuilder)
				{
					SubMenuBuilder.BeginSection("Existing", LOCTEXT("Existing", "Existing"));
					if (UChooserTable* OuterChooser = Cast<UChooserTable>(TransactionObject))
					{
						UChooserTable* RootTable = OuterChooser->GetRootChooser();
						for (UObject* Object : RootTable->NestedObjects)
						{
							if (UStructChooserTable* Nested = Cast<UStructChooserTable>(Object))
							{
								if (Nested == RootTable || !MatchesExpectedOutputStructType(Nested, ExpectedStructType))
								{
									continue;
								}
								SubMenuBuilder.AddMenuEntry(
									FText::FromString(Nested->GetName()),
									LOCTEXT("AddExistingEvaluateTip", "Evaluate this embedded StructChooserTable"),
									FSlateIcon(),
									FUIAction(FExecuteAction::CreateLambda([Nested, NestedPickButton, EvaluateChooser, TransactionObject, ValueChanged]()
									{
										const FScopedTransaction Transaction(LOCTEXT("SetEvaluateNestedStructChooser", "Set Evaluate Struct Chooser"));
										TransactionObject->Modify(true);
										NestedPickButton->SetIsOpen(false);
										EvaluateChooser->Chooser = Nested;
										ValueChanged.ExecuteIfBound();
									})));
							}
						}
					}
					SubMenuBuilder.EndSection();
				}));

			MenuBuilder.EndSection();
			return MenuBuilder.MakeWidget();
		}));

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(SObjectPropertyEntryBox)
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
				if (UStructChooserTable* Other = Cast<UStructChooserTable>(InAssetData.GetAsset()))
				{
					return !MatchesExpectedOutputStructType(Other, ExpectedStructType);
				}
				return true;
			})
			.OnObjectChanged_Lambda([TransactionObject, EvaluateChooser, ValueChanged](const FAssetData& AssetData)
			{
				const FScopedTransaction Transaction(LOCTEXT("EditEvaluateStructChooser", "Edit Evaluate Struct Chooser"));
				TransactionObject->Modify(true);
				EvaluateChooser->Chooser = Cast<UStructChooserTable>(AssetData.GetAsset());
				ValueChanged.ExecuteIfBound();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2.f, 0.f, 0.f, 0.f)
		[
			NestedPickButton
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2.f, 0.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Edit", "Edit"))
			.IsEnabled_Lambda([EvaluateChooser]()
			{
				return EvaluateChooser->Chooser != nullptr;
			})
			.OnClicked_Lambda([EvaluateChooser, TransactionObject]()
			{
				if (EvaluateChooser->Chooser)
				{
					OpenStructChooserForEditing(TransactionObject, EvaluateChooser->Chooser);
				}
				return FReply::Handled();
			})
		];
}

static TSharedRef<SWidget> CreateNestedStructChooserWidget(bool bReadOnly, UObject* TransactionObject, void* Value, UClass* ResultBaseClass, FChooserWidgetValueChanged ValueChanged)
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
				// Match UChooser NestedChooser widget: ChooserEditorStyle has no NestedChooserIcon.
				SNew(SImage).Image(GetChooserEditorBrush(TEXT("ChooserEditor.ChooserTableIconSmall")))
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
			.OnClicked_Lambda([NestedChooser, TransactionObject]()
			{
				if (NestedChooser->Chooser)
				{
					OpenStructChooserForEditing(TransactionObject, NestedChooser->Chooser);
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
		ValueChanged);

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

	Button->SetOnGetMenuContent(FOnGetContent::CreateLambda([WeakTable, ResultData, Button, Border, ValueChanged, NullValueDisplayText]()
	{
		FStructViewerInitializationOptions Options;
		Options.StructFilter = MakeShared<FStructChooserResultFilter>();
		Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
		Options.bShowNoneOption = false;

		FStructViewerModule& StructViewer = FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer");
		return StructViewer.CreateStructViewer(Options, FOnStructPicked::CreateLambda(
			[WeakTable, ResultData, Button, Border, ValueChanged, NullValueDisplayText](const UScriptStruct* ChosenStruct)
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
					ValueChanged);

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
