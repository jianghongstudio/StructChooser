#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * Replaces engine FChooserRowDetails for "ChooserRowDetails".
 * Non-StructChooser tables: identical to engine (hide Chooser only).
 * UStructChooserTable: StructTypeConst on Result + custom Result Type combo
 * (FStructChooserResultFilter) for FStructChooserBase children.
 */
class FStructChooserRowDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
