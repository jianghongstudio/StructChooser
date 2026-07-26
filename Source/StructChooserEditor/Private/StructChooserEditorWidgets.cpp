#include "StructChooserEditorWidgets.h"
#include "StructChooserTypes.h"
#include "StructChooserTable.h"
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
#include "Widgets/SBoxPanel.h"
#include "PropertyCustomizationHelpers.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "StructChooserEditorWidgets"

namespace UE::StructChooserEditor
{
using namespace UE::ChooserEditor;

static const FSlateBrush* GetChooserTableIconSmall()
{
	// ChooserEditorStyle.h is Private to ChooserEditor — look up the registered style by name.
	if (const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(FName(TEXT("ChooserEditorStyle"))))
	{
		return Style->GetBrush(TEXT("ChooserEditor.ChooserTableIconSmall"));
	}
	return FAppStyle::GetBrush(TEXT("ClassIcon.Object"));
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
				SNew(SImage).Image(GetChooserTableIconSmall())
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
					// FChooserTableEditor / PushChooserTableToEdit are Private to ChooserEditor.
					// FocusWindow still switches the open Chooser table editor to the nested object.
					if (UObject* RootChooser = TransactionObject->GetPackage()->FindAssetInPackage())
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
}

#undef LOCTEXT_NAMESPACE
