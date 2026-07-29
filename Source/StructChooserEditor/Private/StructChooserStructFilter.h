#pragma once

#include "CoreMinimal.h"
#include "StructChooserTypes.h"
#include "StructViewerFilter.h"

/**
 * Like engine FStructFilter, but allows Hidden children of FStructChooserBase
 * so StructChooser authoring still works after those types are Meta=(Hidden)
 * (Hidden keeps them out of official ObjectResult Add Row / cell pickers).
 */
class FStructChooserResultFilter : public IStructViewerFilter
{
public:
	virtual bool IsStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const UScriptStruct* InStruct, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		if (!InStruct || InStruct == FStructChooserBase::StaticStruct())
		{
			return false;
		}
		if (!InStruct->IsChildOf(FStructChooserBase::StaticStruct()))
		{
			return false;
		}
		// Intentionally allow Hidden — official menus use engine FStructFilter which rejects them.
		return true;
	}

	virtual bool IsUnloadedStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const FSoftObjectPath& InStructPath, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		return false;
	}
};
