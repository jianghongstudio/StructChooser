#pragma once

#include "CoreMinimal.h"
#include "ObjectChooserWidgetFactories.h"
#include "StructUtils/InstancedStruct.h"

class UChooserTable;

namespace UE::StructChooserEditor
{
	/** Register only StructChooser result cell widgets — never overwrite engine Nested/Asset/Evaluate creators. */
	void RegisterStructChooserWidgets();

	/**
	 * Result cell content + type picker filtered to FStructChooserBase children (allows Hidden).
	 * Do not use FObjectChooserWidgetFactories::CreateWidget with FObjectChooserBase — that lists Asset/Class.
	 */
	TSharedPtr<SWidget> CreateStructChooserResultCellWidget(
		bool bReadOnly,
		UChooserTable* TransactionObject,
		FInstancedStruct* ResultData,
		UE::ChooserEditor::FChooserWidgetValueChanged ValueChanged,
		UE::ChooserEditor::IChooserTableWidgetInterface* ChooserWidgetInterface,
		TSharedPtr<SBorder>* InnerWidget = nullptr,
		FText NullValueDisplayText = FText());
}
