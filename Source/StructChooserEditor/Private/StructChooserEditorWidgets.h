#pragma once

#include "CoreMinimal.h"
#include "ObjectChooserWidgetFactories.h"

namespace UE::StructChooserEditor
{
	void RegisterStructChooserWidgets();

	/** Replace engine Object result cell widgets on UStructChooserTable so invalid types cannot crash. */
	void RegisterObjectResultCrashGuards();
}
