// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Framework/Commands/UICommandList.h"
#include "SStructChooserTableRow.h"
#include "StructChooserTableEditor.h"

class SPositiveActionButton;

namespace UE::StructChooserEditor
{
	
class SStructChooserCreateRowButton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStructChooserCreateRowButton)
	{}
	SLATE_ARGUMENT(TSharedPtr<FStructChooserTableViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SStructChooserCreateRowButton() override;

 private:
	TSharedPtr<FStructChooserTableViewModel> ChooserViewModel;
	TSharedRef<SWidget>	MakeCreateRowMenu();
};

}
