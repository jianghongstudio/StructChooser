// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Framework/Commands/UICommandList.h"
#include "StructChooserTableViewModel.h"
#include "IChooserTableView.h"

class SPositiveActionButton;

namespace UE::StructChooserEditor
{
	
class SStructChooserTableWidget : public IChooserTableView
{
public:
	SLATE_BEGIN_ARGS(SStructChooserTableWidget)
	{}
	SLATE_ARGUMENT(TSharedPtr<FUICommandList>, Commands)
	SLATE_ARGUMENT(TSharedPtr<IChooserTableViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SStructChooserTableWidget() override;
	
	void SelectRows(const TConstArrayView<int32> Rows);
	void SelectRow(int32 RowIndex, bool bClear = true);

	void UpdateTableColumns();
	void UpdateTableRows();

	void ClearSelection();
	void GetSelectedItems(TArray<TSharedPtr<FStructChooserTableRow>> &SelectedItems);

	int GetSelectedColumn() const;

	bool TableHasFocus() const;
	
	// Command handler functions
	
	bool HasSelection() const;
	bool HasRowsSelected() const;
	bool HasColumnSelected() const;
	
	void SelectRootProperties();
	
	bool CanAutoPopulateSelection() const;
	void AutoPopulateSelection();
	void AutoPopulateAll();
	
	void RemoveDisabledData();
	void DeleteSelection();
	void DuplicateSelection();
	
	bool IsSelectionDisabled() const;
	void ToggleDisableSelection();
	
	void CopySelection();
	void CutSelection();
	bool CanPaste() const;
	void Paste();
	
	bool CanMoveRowsUp();
	void MoveRowsUp();
	bool CanMoveRowsDown();
	void MoveRowsDown();
	bool CanMoveColumnLeft();
	void MoveColumnLeft();
	bool CanMoveColumnRight();
	void MoveColumnRight();

	
	virtual TSharedPtr<IChooserTableViewModel> GetViewModel() override { return StaticCastSharedPtr<IChooserTableViewModel>(ChooserViewModel); };
	
 private:
	void BindCommands(TSharedPtr<FUICommandList> CommandList);

	TSharedPtr<SListView<TSharedPtr<FStructChooserTableRow>>> TableView;
	TSharedPtr<SHeaderRow> HeaderRow;
	
	TSharedPtr<SWidget> CreateColumnComboButton;
	TSharedPtr<SWidget> GenerateRowContextMenu();
	TSharedRef<ITableRow> GenerateTableRow(TSharedPtr<FStructChooserTableRow> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FStructChooserTableViewModel> ChooserViewModel;
	TArray<TSharedPtr<FStructChooserTableRow>> TableRows;
};

}
