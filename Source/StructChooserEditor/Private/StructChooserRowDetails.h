#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * Replaces engine FChooserRowDetails for "ChooserRowDetails".
 * On UStructChooserTable rows, narrows Result InstancedStruct BaseStruct to FStructChooserBase.
 */
class FStructChooserRowDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
