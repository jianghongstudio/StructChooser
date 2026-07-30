// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "IDetailCustomization.h"
#include "StructUtils/PropertyBag.h"
#include "StructChooserEditorDetails.generated.h"

class UChooserTable;

UCLASS()
class UStructChooserRowDetails : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION()
	bool ResultAssetFilter(const FAssetData& AssetData);

	UPROPERTY(EditAnywhere, Category="Properties", meta=(FixedLayout, ShowOnlyInnerProperties))
	FInstancedPropertyBag Properties;

	UPROPERTY(EditAnywhere, Instanced, Category="Hidden")
	TObjectPtr<UChooserTable> Chooser;
	int Row = INDEX_NONE;
};

UCLASS()
class UStructChooserColumnDetails : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Instanced, Category="Hidden")
	TObjectPtr<UChooserTable> Chooser;
	int32 Column = -1;
};

namespace UE::StructChooserEditor
{
	class FStructChooserColumnDetails : public IDetailCustomization
	{
	public:
		static TSharedRef<IDetailCustomization> MakeInstance()
		{
			return MakeShareable(new FStructChooserColumnDetails());
		}

		virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
	};
}
