// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

class UChooserTable;

namespace UE::StructChooserEditor
{

class FStructChooserTableViewModel;

struct FStructChooserTableRow
{
	FStructChooserTableRow(int32 i) { RowIndex = i; }
	int32 RowIndex;
};

class SStructChooserTableRow : public SMultiColumnTableRow<TSharedPtr<FStructChooserTableRow>>
{
public:
SLATE_BEGIN_ARGS(SStructChooserTableRow) {}
/** The list item for this row */
	SLATE_ARGUMENT(TSharedPtr<FStructChooserTableRow>, Entry)
	SLATE_ARGUMENT(UChooserTable*, Chooser)
	SLATE_ARGUMENT(TSharedPtr<FStructChooserTableViewModel>, ViewModel)
	SLATE_ATTRIBUTE(bool, TableHasFocus);
	SLATE_END_ARGS()

	enum { SpecialIndex_AddRow = -1, SpecialIndex_Fallback = -2 };

	void Construct(const FArguments& Args, const TSharedRef<STableViewBase>& OwnerTableView);

	/** Overridden from SMultiColumnTableRow.  Generates a widget for this column of the list view. */
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TAttribute<bool> TableHasFocus;
	TSharedPtr<FStructChooserTableRow> RowIndex;
	UChooserTable* Chooser;
	TSharedPtr<FStructChooserTableViewModel> ChooserViewModel;
	TSharedPtr<SBorder> CacheBorder;
	int DragActiveCounter = 0;
	bool bDragActive = false;
	bool bDropSupported = false;
	bool bDropAbove = false;
};

}
