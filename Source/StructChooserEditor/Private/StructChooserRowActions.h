#pragma once

#include "CoreMinimal.h"
#include "IChooserTableViewModel.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

class UScriptStruct;
class UStructChooserTable;

namespace UE::StructChooserEditor
{
	/** Add a Struct / Evaluate / Nested result row (aligns DisabledRows, Compile). */
	void AddStructResultRow(UStructChooserTable* Table, const UScriptStruct* ResultType);

	/** Set FallbackResult to a StructChooser result type. */
	void AddStructFallbackResult(UStructChooserTable* Table, const UScriptStruct* ResultType);

	/** Build Add Row menu content for StructChooser only (no Asset/Class/Object types). */
	TSharedRef<SWidget> MakeStructChooserCreateRowMenu(
		TWeakObjectPtr<UStructChooserTable> WeakRootTable,
		TWeakPtr<UE::ChooserEditor::IChooserTableViewModel> WeakViewModel);

	/** Cache ViewModel for a root StructChooser so Add Row patch can RefreshAll. */
	void CacheStructChooserViewModel(
		UStructChooserTable* RootTable,
		TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> ViewModel);

	TSharedPtr<UE::ChooserEditor::IChooserTableViewModel> FindCachedStructChooserViewModel(UStructChooserTable* AnyTable);

	void ClearCachedStructChooserViewModels();
}
