#include "StructChooserDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"

TSharedRef<IDetailCustomization> FStructChooserDetails::MakeInstance()
{
	return MakeShareable(new FStructChooserDetails);
}

void FStructChooserDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Placeholder Object Result fields — StructChooser uses OutputStructType instead.
	DetailBuilder.HideProperty(TEXT("ResultType"));
	DetailBuilder.HideProperty(TEXT("OutputObjectType"));
	if (TSharedPtr<IPropertyHandle> ResultTypeHandle = DetailBuilder.GetProperty(TEXT("ResultType")))
	{
		ResultTypeHandle->MarkHiddenByCustomization();
	}
	if (TSharedPtr<IPropertyHandle> OutputObjectHandle = DetailBuilder.GetProperty(TEXT("OutputObjectType")))
	{
		OutputObjectHandle->MarkHiddenByCustomization();
	}

	// Match FChooserDetails: keep Results/Columns EditAnywhere for row/column selection,
	// but hide them on the root Table Settings view.
	IDetailCategoryBuilder& HiddenCategory = DetailBuilder.EditCategory(TEXT("Hidden"));
	TArray<TSharedRef<IPropertyHandle>> HiddenProperties;
	HiddenCategory.GetDefaultProperties(HiddenProperties);
	for (TSharedRef<IPropertyHandle>& PropertyHandle : HiddenProperties)
	{
		PropertyHandle->MarkHiddenByCustomization();
	}
}
