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
	// ResultType / OutputObjectType live on UChooserSignature (not UStructChooserTable).
	// Must pass owning class or HideProperty won't find them.
	DetailBuilder.HideProperty(
		GET_MEMBER_NAME_CHECKED(UChooserSignature, ResultType),
		UChooserSignature::StaticClass());
	DetailBuilder.HideProperty(
		GET_MEMBER_NAME_CHECKED(UChooserSignature, OutputObjectType),
		UChooserSignature::StaticClass());

	if (TSharedPtr<IPropertyHandle> ResultTypeHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UChooserSignature, ResultType), UChooserSignature::StaticClass()))
	{
		ResultTypeHandle->MarkHiddenByCustomization();
	}
	if (TSharedPtr<IPropertyHandle> OutputObjectHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UChooserSignature, OutputObjectType), UChooserSignature::StaticClass()))
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
