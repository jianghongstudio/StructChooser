#include "StructChooserDetails.h"
#include "ChooserSignature.h"
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
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UChooserSignature, ResultType));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UChooserSignature, OutputObjectType));

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
