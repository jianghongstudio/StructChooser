#pragma once

#include "StructChooserTable.h"
#include "AssetDefinitionDefault.h"
#include "AssetDefinition_StructChooserTable.generated.h"

UCLASS()
class UAssetDefinition_StructChooserTable : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override
	{
		return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_StructChooserTable", "Struct Chooser Table");
	}

	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor(64, 128, 160)); }

	virtual TSoftClassPtr<UObject> GetAssetClass() const override
	{
		return UStructChooserTable::StaticClass();
	}

	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const TArray<FAssetCategoryPath> Categories = {
			FAssetCategoryPath(EAssetCategoryPaths::Animation),
			FAssetCategoryPath(EAssetCategoryPaths::Misc)
		};
		return Categories;
	}

	virtual EAssetCommandResult OpenAssets(const FAssetOpenArgs& OpenArgs) const override;
};
