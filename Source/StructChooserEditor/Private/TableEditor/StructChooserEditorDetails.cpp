// Copyright Epic Games, Inc. All Rights Reserved.

#include "StructChooserEditorDetails.h"

#include "StructChooserTableEditor.h"

#include "Chooser.h"
#include "ChooserPropertyAccess.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailsView.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StructChooserEditorDetails)

#define LOCTEXT_NAMESPACE "ChooserDetails"

bool UStructChooserRowDetails::ResultAssetFilter(const FAssetData& AssetData)
{
	if (Chooser)
	{
		return Chooser->ResultAssetFilter(AssetData);
	}
	return true;
}

namespace UE::StructChooserEditor
{
void FStructChooserColumnDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	check(!Objects.IsEmpty());

	UStructChooserColumnDetails* Column = Cast<UStructChooserColumnDetails>(Objects[0]);
	UChooserTable* Chooser = Column->Chooser;
	
	if (Chooser->ColumnsStructs.IsValidIndex(Column->Column))
	{
		IDetailCategoryBuilder& PropertiesCategory = DetailBuilder.EditCategory("Column Properties");

		TSharedPtr<IPropertyHandle> ChooserProperty = DetailBuilder.GetProperty("Chooser", Column->StaticClass());
		DetailBuilder.HideProperty(ChooserProperty);
	
		TSharedPtr<IPropertyHandle> ColumnsArrayProperty = ChooserProperty->GetChildHandle("ColumnsStructs");
		TSharedPtr<IPropertyHandle> CurrentColumnProperty = ColumnsArrayProperty->AsArray()->GetElement(Column->Column);

		IDetailPropertyRow& NewColumnProperty = PropertiesCategory.AddProperty(CurrentColumnProperty);
		// hide array add button
		NewColumnProperty.ShowPropertyButtons(false);
		// removing the column type, and just showing the column instance struct properties
		NewColumnProperty.CustomWidget(true);
	}
}

}

#undef LOCTEXT_NAMESPACE
