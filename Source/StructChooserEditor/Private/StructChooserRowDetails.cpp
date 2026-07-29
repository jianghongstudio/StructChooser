#include "StructChooserRowDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"
#include "StructChooserStructFilter.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"
#include "StructUtils/PropertyBag.h"
#include "StructViewerModule.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "StructChooserRowDetails"

namespace
{
	static const FName BaseStructMetaName(TEXT("BaseStruct"));
	static const FName StructTypeConstMetaName(TEXT("StructTypeConst"));
	static const FName ChooserPropName(TEXT("Chooser"));
	static const FName PropertiesPropName(TEXT("Properties"));
	static const FName ResultPropName(TEXT("Result"));
	// Matches UE::ChooserEditor::ColumnWidget_SpecialIndex_Fallback
	static constexpr int32 FallbackRowIndex = -2;

	static FText GetResultTypeLabel(const FInstancedStruct& Result)
	{
		if (const UScriptStruct* Type = Result.GetScriptStruct())
		{
			return Type->GetDisplayNameText();
		}
		return LOCTEXT("NoResultType", "Select Result Type...");
	}

	static FInstancedStruct* GetRowResultMutable(UStructChooserTable* Table, int32 RowIndex)
	{
		if (!Table)
		{
			return nullptr;
		}
		if (RowIndex == FallbackRowIndex)
		{
			return &Table->FallbackResult;
		}
		if (Table->ResultsStructs.IsValidIndex(RowIndex))
		{
			return &Table->ResultsStructs[RowIndex];
		}
		return nullptr;
	}

	static void MakeDefaultResultOfType(UStructChooserTable* Table, const UScriptStruct* NewType, FInstancedStruct& OutResult)
	{
		if (NewType == FStructValueChooser::StaticStruct())
		{
			OutResult.InitializeAs(FStructValueChooser::StaticStruct());
			UScriptStruct* OutputType = nullptr;
			if (UStructChooserTable* Root = Cast<UStructChooserTable>(Table->GetRootChooser()))
			{
				OutputType = Root->OutputStructType;
			}
			else
			{
				OutputType = Table->OutputStructType;
			}
			if (OutputType)
			{
				OutResult.GetMutable<FStructValueChooser>().Value.InitializeAs(OutputType);
			}
		}
		else
		{
			OutResult.InitializeAs(NewType);
		}
	}
}

TSharedRef<IDetailCustomization> FStructChooserRowDetails::MakeInstance()
{
	return MakeShareable(new FStructChooserRowDetails);
}

void FStructChooserRowDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.IsEmpty() || !Objects[0].IsValid())
	{
		return;
	}

	UObject* RowObject = Objects[0].Get();
	UClass* RowClass = RowObject->GetClass();
	if (!RowClass || RowClass->GetFName() != FName(TEXT("StructChooserRowDetails")))
	{
		return;
	}

	// Match engine FChooserRowDetails — hide Chooser (ChooserEditor Private type, use reflection).
	TSharedPtr<IPropertyHandle> ChooserProperty = DetailBuilder.GetProperty(ChooserPropName, RowClass);
	DetailBuilder.HideProperty(ChooserProperty);

	const FObjectProperty* ChooserProp = FindFProperty<FObjectProperty>(RowClass, ChooserPropName);
	const FStructProperty* PropertiesProp = FindFProperty<FStructProperty>(RowClass, PropertiesPropName);
	const FIntProperty* RowProp = FindFProperty<FIntProperty>(RowClass, TEXT("Row"));
	if (!ChooserProp || !PropertiesProp || !RowProp)
	{
		return;
	}

	UChooserTable* Chooser = Cast<UChooserTable>(ChooserProp->GetObjectPropertyValue_InContainer(RowObject));
	UStructChooserTable* StructChooser = Cast<UStructChooserTable>(Chooser);
	if (!StructChooser)
	{
		return;
	}

	FInstancedPropertyBag& Properties = *PropertiesProp->ContainerPtrToValuePtr<FInstancedPropertyBag>(RowObject);
	const FPropertyBagPropertyDesc* ResultDesc = Properties.FindPropertyDescByName(ResultPropName);
	if (!ResultDesc)
	{
		return;
	}

	const int32 RowIndex = RowProp->GetPropertyValue_InContainer(RowObject);

	const FString DesiredBase = FStructChooserBase::StaticStruct()->GetPathName();
	const bool bNeedsMetaUpdate =
		ResultDesc->GetMetaData(BaseStructMetaName) != DesiredBase ||
		!ResultDesc->HasMetaData(StructTypeConstMetaName);

	if (bNeedsMetaUpdate)
	{
		FPropertyBagPropertyDesc NewDesc = *ResultDesc;
		NewDesc.SetMetaData(BaseStructMetaName, DesiredBase);
		// Lock InstancedStruct type picker (Hidden types would be empty anyway); we provide our own combo.
		NewDesc.SetMetaData(StructTypeConstMetaName, FString());
		Properties.AddProperties({NewDesc}, /*bOverwrite=*/true);

		if (TSharedPtr<IPropertyUtilities> Utils = DetailBuilder.GetPropertyUtilities())
		{
			Utils->RequestForceRefresh();
		}
		return;
	}

	IDetailCategoryBuilder& TypeCategory = DetailBuilder.EditCategory(
		TEXT("StructChooserResultType"),
		LOCTEXT("StructChooserResultTypeCat", "Result Type"),
		ECategoryPriority::Important);

	TWeakObjectPtr<UStructChooserTable> WeakTable = StructChooser;
	TWeakObjectPtr<UObject> WeakRowObject = RowObject;
	TSharedPtr<IPropertyUtilities> PropertyUtils = DetailBuilder.GetPropertyUtilities();

	TypeCategory.AddCustomRow(LOCTEXT("ResultTypeRow", "Result Type"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ResultTypeLabel", "Result Type"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	.MinDesiredWidth(250.f)
	[
		SNew(SComboButton)
		.ContentPadding(FMargin(2, 2))
		.ButtonContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text_Lambda([WeakTable, RowIndex]()
			{
				UStructChooserTable* Table = WeakTable.Get();
				if (const FInstancedStruct* Result = GetRowResultMutable(Table, RowIndex))
				{
					return GetResultTypeLabel(*Result);
				}
				return LOCTEXT("NoResultType", "Select Result Type...");
			})
		]
		.OnGetMenuContent_Lambda([WeakTable, WeakRowObject, RowIndex, PropertyUtils]()
		{
			FStructViewerInitializationOptions Options;
			Options.StructFilter = MakeShared<FStructChooserResultFilter>();
			Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
			Options.bShowNoneOption = false;

			FStructViewerModule& StructViewer = FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer");
			return StructViewer.CreateStructViewer(Options, FOnStructPicked::CreateLambda(
				[WeakTable, WeakRowObject, RowIndex, PropertyUtils](const UScriptStruct* ChosenStruct)
				{
					UStructChooserTable* Table = WeakTable.Get();
					UObject* RowObject = WeakRowObject.Get();
					if (!Table || !RowObject || !ChosenStruct)
					{
						return;
					}

					FInstancedStruct* Target = GetRowResultMutable(Table, RowIndex);
					const FStructProperty* PropsProp = FindFProperty<FStructProperty>(RowObject->GetClass(), PropertiesPropName);
					if (!Target || !PropsProp)
					{
						return;
					}

					const FScopedTransaction Transaction(LOCTEXT("ChangeStructResultType", "Change Struct Chooser Result Type"));
					Table->Modify(true);
					MakeDefaultResultOfType(Table, ChosenStruct, *Target);

					FInstancedPropertyBag& Props = *PropsProp->ContainerPtrToValuePtr<FInstancedPropertyBag>(RowObject);
					Props.SetValueStruct(ResultPropName, FConstStructView(FInstancedStruct::StaticStruct(), reinterpret_cast<uint8*>(Target)));

					if (PropertyUtils.IsValid())
					{
						PropertyUtils->RequestForceRefresh();
					}
				}));
		})
	];
}

#undef LOCTEXT_NAMESPACE
