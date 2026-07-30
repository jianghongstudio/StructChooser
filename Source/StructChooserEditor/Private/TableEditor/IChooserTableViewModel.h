// Copyright Epic Games, Inc. All Rights Reserved.
//
// UE 5.7 shims for ChooserEditor public APIs that exist on UE 5.8 (main StructChooser)
// but are absent from the current engine. Keep signatures aligned with main's usage.

#pragma once

#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "Templates/SharedPointer.h"

class UChooserTable;
class UObject;

namespace UE::ChooserEditor
{
	DECLARE_DELEGATE_OneParam(FOpenObject, UObject*);
	DECLARE_DELEGATE_OneParam(FShowDetails, const TArray<UObject*>&);

	/** Nested / Evaluate cell Edit — push nested table into the open StructChooser editor. */
	class IChooserTableWidgetInterface
	{
	public:
		virtual ~IChooserTableWidgetInterface() = default;
		virtual void OpenObject(UObject* Object) = 0;
	};

	/**
	 * ViewModel surface used by StructChooser's dedicated table editor (mirrors UE5.8 ChooserEditor).
	 * Inherits FNotifyHook so DetailsViewArgs.NotifyHook can bind to the ViewModel.
	 */
	class IChooserTableViewModel : public FNotifyHook
	{
	public:
		virtual ~IChooserTableViewModel() = default;

		virtual void SetOpenObjectDelegate(FOpenObject InOpenObjectDelegate) = 0;
		virtual void SetShowDetailsDelegate(FShowDetails InShowDetailsDelegate) = 0;
		virtual void RegisterMenus(TSharedPtr<class FUICommandList> CommandList) = 0;

		virtual const UChooserTable* GetRootChooser() const = 0;
		virtual UChooserTable* GetRootChooser() = 0;
		virtual UChooserTable* GetChooser() = 0;
		virtual const UChooserTable* GetChooser() const = 0;

		virtual void RefreshAll() = 0;
		virtual void SetChooser(UChooserTable* Chooser) = 0;
		virtual void AutoPopulateAll() = 0;
		virtual void SelectRootProperties() = 0;
	};
}
