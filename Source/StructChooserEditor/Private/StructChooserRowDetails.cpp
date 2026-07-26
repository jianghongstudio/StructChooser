#include "StructChooserRowDetails.h"

#include "DetailLayoutBuilder.h"
#include "IPropertyUtilities.h"
#include "StructChooserTable.h"
#include "StructChooserTypes.h"
#include "StructUtils/PropertyBag.h"

namespace
{
	static const FName BaseStructMetaName(TEXT("BaseStruct"));
	static const FName ChooserPropName(TEXT("Chooser"));
	static const FName PropertiesPropName(TEXT("Properties"));
	static const FName ResultPropName(TEXT("Result"));
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
	if (!RowClass || RowClass->GetFName() != FName(TEXT("ChooserRowDetails")))
	{
		return;
	}

	// Match engine FChooserRowDetails — hide Chooser (ChooserEditor Private type, use reflection).
	TSharedPtr<IPropertyHandle> ChooserProperty = DetailBuilder.GetProperty(ChooserPropName, RowClass);
	DetailBuilder.HideProperty(ChooserProperty);

	const FObjectProperty* ChooserProp = FindFProperty<FObjectProperty>(RowClass, ChooserPropName);
	const FStructProperty* PropertiesProp = FindFProperty<FStructProperty>(RowClass, PropertiesPropName);
	if (!ChooserProp || !PropertiesProp)
	{
		return;
	}

	UChooserTable* Chooser = Cast<UChooserTable>(ChooserProp->GetObjectPropertyValue_InContainer(RowObject));
	if (!Cast<UStructChooserTable>(Chooser))
	{
		return;
	}

	FInstancedPropertyBag& Properties = *PropertiesProp->ContainerPtrToValuePtr<FInstancedPropertyBag>(RowObject);
	const FPropertyBagPropertyDesc* ResultDesc = Properties.FindPropertyDescByName(ResultPropName);
	if (!ResultDesc)
	{
		return;
	}

	const FString DesiredBase = FStructChooserBase::StaticStruct()->GetPathName();
	if (ResultDesc->GetMetaData(BaseStructMetaName) == DesiredBase)
	{
		return;
	}

	FPropertyBagPropertyDesc NewDesc = *ResultDesc;
	NewDesc.SetMetaData(BaseStructMetaName, DesiredBase);
	Properties.AddProperties({NewDesc}, /*bOverwrite=*/true);

	// Deferred refresh so the InstancedStruct picker rebuilds with the new BaseStruct.
	if (TSharedPtr<IPropertyUtilities> Utils = DetailBuilder.GetPropertyUtilities())
	{
		Utils->RequestForceRefresh();
	}
}
