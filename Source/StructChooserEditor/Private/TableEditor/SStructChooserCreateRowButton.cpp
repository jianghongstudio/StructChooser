// Copyright Epic Games, Inc. All Rights Reserved.

#include "SStructChooserCreateRowButton.h"
#include "IChooserTableViewModel.h"
#include "StructChooserRowActions.h"
#include "StructChooserTable.h"
#include "SPositiveActionButton.h"

#define LOCTEXT_NAMESPACE "StructChooserAddRowButton"

namespace UE::StructChooserEditor
{

TSharedRef<SWidget> SStructChooserCreateRowButton::MakeCreateRowMenu()
{
	UStructChooserTable* Table = Cast<UStructChooserTable>(ChooserViewModel->GetChooser());
	TWeakObjectPtr<UStructChooserTable> WeakTable = Table;
	TWeakPtr<UE::ChooserEditor::IChooserTableViewModel> WeakViewModel = ChooserViewModel;
	return MakeStructChooserCreateRowMenu(WeakTable, WeakViewModel);
}

void SStructChooserCreateRowButton::Construct(const FArguments& InArgs)
{
	ChooserViewModel = InArgs._ViewModel;
	check(ChooserViewModel);

	ChildSlot
	[
		SNew(SPositiveActionButton)
			.Text(LOCTEXT("Add Row", "Add Row"))
			.OnGetMenuContent(this, &SStructChooserCreateRowButton::MakeCreateRowMenu)
	];
}

SStructChooserCreateRowButton::~SStructChooserCreateRowButton()
{
}

}

#undef LOCTEXT_NAMESPACE
