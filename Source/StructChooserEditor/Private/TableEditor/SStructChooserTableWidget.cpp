// Copyright Epic Games, Inc. All Rights Reserved.

#include "SStructChooserTableWidget.h"
#include "StructChooserTableEditor.h"
#include "Chooser.h"
#include "Framework/Application/SlateApplication.h"
#include "ObjectTools.h"
#include "OutputObjectColumn.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "ObjectChooserWidgetFactories.h"
#include "SStructChooserCreateColumnButton.h"
#include "SStructChooserColumnHandle.h"
#include "SStructChooserTableRow.h"
#include "ToolMenus.h"
#include "ChooserTableEditorCommands.h"
#include "Framework/Commands/GenericCommands.h"


#define LOCTEXT_NAMESPACE "ChooserTableWidget"

namespace UE::StructChooserEditor
{
	
	bool SStructChooserTableWidget::TableHasFocus() const
	{
		if (TableView)
		{
			return TableView->HasKeyboardFocus();
		}
		return false;
	}
	
	void SStructChooserTableWidget::ClearSelection()
	{
		if (TableView)
		{
			TableView->ClearSelection();
		}
	}
	
	void SStructChooserTableWidget::GetSelectedItems(TArray<TSharedPtr<FStructChooserTableRow>>& SelectedItems)
	{
		TableView->GetSelectedItems(SelectedItems);
	}

	int SStructChooserTableWidget::GetSelectedColumn() const
	{
		if (ChooserViewModel)
		{
			return ChooserViewModel->GetSelectedColumn();
		}
		return INDEX_NONE;
	}
	
	void SStructChooserTableWidget::UpdateTableColumns()
	{
		if (ChooserViewModel)
		{
			UChooserTable* Chooser = ChooserViewModel->GetChooser();

			HeaderRow->ClearColumns();

			HeaderRow->AddColumn(SHeaderRow::Column("Handles")
							.DefaultLabel(FText())
							.ManualWidth(30));

			if (Chooser->ResultType != EObjectChooserResultType::NoPrimaryResult)
			{
				HeaderRow->AddColumn(SHeaderRow::Column("Result")
								.ManualWidth_Lambda([Chooser]() { return Chooser->EditorResultsColumnWidth; } )
								.OnWidthChanged_Lambda([Chooser](float NewWidth) { Chooser->EditorResultsColumnWidth = NewWidth; })
								.HeaderContent()
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.VAlign(VAlign_Top)
									[
										SNew(STextBlock)
											.Text(LOCTEXT("Result", "Result"))
											.ToolTipText(LOCTEXT("ResultTooltip", "The Result is the asset which will be returned if a row is selected (or other Chooser to evaluate to get the asset to return"))
									]
								]);
			}

			FName ColumnId("ChooserColumn", 1);
			int NumColumns = Chooser->ColumnsStructs.Num();	
			for(int ColumnIndex = 0; ColumnIndex < NumColumns; ColumnIndex++)
			{
				FChooserColumnBase& Column = Chooser->ColumnsStructs[ColumnIndex].GetMutable<FChooserColumnBase>();

				TSharedPtr<SWidget> HeaderWidget = FObjectChooserWidgetFactories::CreateColumnWidget(&Column, Chooser->ColumnsStructs[ColumnIndex].GetScriptStruct(), Chooser->GetRootChooser(), -1);
				if (!HeaderWidget.IsValid())
				{
					HeaderWidget = SNullWidget::NullWidget;
				}
				
				HeaderRow->AddColumn(SHeaderRow::FColumn::FArguments()
					.ColumnId(ColumnId)
					.ManualWidth(Column.EditorColumnWidth)
					.ManualWidth_Lambda([&Column]() { return Column.EditorColumnWidth; } )
					.OnWidthChanged_Lambda([&Column](float NewWidth) { Column.EditorColumnWidth = NewWidth; })
					.HeaderComboVisibility(EHeaderComboVisibility::Ghosted)
					.HeaderContent()
					[
						SNew(SStructChooserColumnHandle)
							.ViewModel(ChooserViewModel)
							.TableHasFocus_Raw(this, &SStructChooserTableWidget::TableHasFocus)
							.ColumnIndex(ColumnIndex)
							.NoDropAfter(Chooser->ColumnsStructs[ColumnIndex].Get<FChooserColumnBase>().IsRandomizeColumn())
						[
							HeaderWidget.ToSharedRef()
						]
					
					]);
			
				ColumnId.SetNumber(ColumnId.GetNumber() + 1);
			}

			HeaderRow->AddColumn( SHeaderRow::FColumn::FArguments()
				.ColumnId("Add")
				.FillWidth(1.0)
				.HeaderContent( )
				[
					SNew(SVerticalBox)
					 + SVerticalBox::Slot().AutoHeight()
					 [
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().MaxWidth(150)
						[
							CreateColumnComboButton.ToSharedRef()
						]
					]
				]
				);
		}
	}

	void SStructChooserTableWidget::UpdateTableRows()
	{
		UChooserTable* Chooser = ChooserViewModel->GetChooser();

		int32 NewNum = Chooser->ResultsStructs.Num();
		Chooser->DisabledRows.SetNum(NewNum);
    
		// Sync the TableRows array which drives the ui table to match the number of results.
		TableRows.SetNum(0, EAllowShrinking::No);
		for(int i =0; i < NewNum; i++)
		{
			TableRows.Add(MakeShared<FStructChooserTableRow>(i));
		}
    
		// Add one at the end, for the Fallback result
		if (Chooser->FallbackResult.IsValid())
		{
			TableRows.Add(MakeShared<FStructChooserTableRow>(SStructChooserTableRow::SpecialIndex_Fallback));
		}
    	
		// Add one at the end, for the "Add Row" control
		TableRows.Add(MakeShared<FStructChooserTableRow>(SStructChooserTableRow::SpecialIndex_AddRow));
    
		// Make sure each column has the same number of row datas as there are results
		for(FInstancedStruct& ColumnData : Chooser->ColumnsStructs)
		{
			FChooserColumnBase& Column = ColumnData.GetMutable<FChooserColumnBase>();
			Column.SetNumRows(NewNum);
		}
    
		if (TableView.IsValid())
		{
			TableView->RebuildList();
		}
	}
			
	TSharedPtr<SWidget> SStructChooserTableWidget::GenerateRowContextMenu()
	{
		UToolMenus* ToolMenus = UToolMenus::Get();
		FToolMenuContext ToolMenuContext;
		ToolMenuContext.AppendCommandList(CommandList);
		return ToolMenus->GenerateWidget(FStructChooserTableEditor::ContextMenuName, ToolMenuContext);
	}
	
	TSharedRef<ITableRow> SStructChooserTableWidget::GenerateTableRow(TSharedPtr<FStructChooserTableRow> InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		UChooserTable* Chooser = ChooserViewModel->GetChooser();

		return SNew(SStructChooserTableRow, OwnerTable)
			.Entry(InItem).Chooser(Chooser).ViewModel(ChooserViewModel).TableHasFocus(this, &SStructChooserTableWidget::TableHasFocus);
	}

	void SStructChooserTableWidget::SelectRow(int32 RowIndex, bool bClear)
    {
    	if (TSharedPtr<FStructChooserTableRow>* Row = TableRows.FindByPredicate([RowIndex](const TSharedPtr<FStructChooserTableRow>& InRow)
    		{
    			return InRow->RowIndex == RowIndex;
    		}))
    	{
    		if (!TableView->IsItemSelected(*Row))
    		{
    			if (bClear)
    			{
    				TableView->ClearSelection();
    			}
    			TableView->SetItemSelection(*Row, true, ESelectInfo::OnMouseClick);
    		}
    	}
    }
    	
    void SStructChooserTableWidget::SelectRows(const TConstArrayView<int32> Rows)
    {
    	
    	TableView->ClearSelection();
    	TArray<TSharedPtr<FStructChooserTableRow>> RowsToSelect;
    
    	for (TSharedPtr<FStructChooserTableRow>& Row : TableRows)
    	{
    		if (Rows.Contains(Row->RowIndex))
    		{
    			RowsToSelect.Add(Row);
    		}
    	}
    	
    	TableView->SetItemSelection(RowsToSelect, true);
    }
	
	
	void SStructChooserTableWidget::Construct(const FArguments& InArgs)
	{
		ChooserViewModel = StaticCastSharedPtr<FStructChooserTableViewModel>(InArgs._ViewModel);
		check(ChooserViewModel);

		BindCommands(InArgs._Commands);

		ChooserViewModel->RefreshTableRowsDelegate = FRefreshTable::CreateLambda([WeakPtr = AsWeak()]()
		{
			if (TSharedPtr<SWidget> Ptr = WeakPtr.Pin())
			{
				StaticCastSharedPtr<SStructChooserTableWidget>(Ptr)->UpdateTableRows();
			}
		});

		ChooserViewModel->RefreshTableColumnsDelegate = FRefreshTable::CreateLambda([WeakPtr = AsWeak()]()
		{
			if (TSharedPtr<SWidget> Ptr = WeakPtr.Pin())
			{
				StaticCastSharedPtr<SStructChooserTableWidget>(Ptr)->UpdateTableColumns();
			}
		});

		ChooserViewModel->SelectRowsDelegate = FSelectRows::CreateLambda([WeakPtr = AsWeak()](TConstArrayView<int32> Rows)
		{
			if (TSharedPtr<SWidget> Ptr = WeakPtr.Pin())
			{
				StaticCastSharedPtr<SStructChooserTableWidget>(Ptr)->SelectRows(Rows);
			}
		});

		CreateColumnComboButton = SNew(SStructChooserCreateColumnButton).ViewModel(ChooserViewModel);
		
		HeaderRow = SNew(SHeaderRow);
		UpdateTableColumns();

		TSharedRef VerticalScrollbar = SNew(SScrollBar);
		
		TableView = SNew(SListView<TSharedPtr<FStructChooserTableRow>>)
					.OnKeyDownHandler_Lambda([Commands = InArgs._Commands](const FGeometry&, const FKeyEvent& Event)
					{
						if (Commands)
						{
							if (Commands->ProcessCommandBindings(Event))
							{
								return FReply::Handled();
							}
						}
						return FReply::Unhandled();
					})
					.ListItemsSource(&TableRows)
					.OnContextMenuOpening(this, &SStructChooserTableWidget::GenerateRowContextMenu)
					.OnGenerateRow(this, &SStructChooserTableWidget::GenerateTableRow)
					.HeaderRow(HeaderRow)
					.ConsumeMouseWheel(EConsumeMouseWheel::Always)
					.ExternalScrollbar(VerticalScrollbar)
					.OnSelectionChanged_Lambda([this](TSharedPtr<FStructChooserTableRow>,  ESelectInfo::Type SelectInfo)
    				{
						TArray<TSharedPtr<FStructChooserTableRow>> SelectedItems;
						GetSelectedItems(SelectedItems);
						TArray<int32> SelectedRows;
						for (TSharedPtr<FStructChooserTableRow> RowItem : SelectedItems)
						{
							SelectedRows.Add(RowItem->RowIndex);
						}
						ChooserViewModel->SetSelectedRows(SelectedRows);
    				});

		ChildSlot
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SScrollBox).Orientation(Orient_Horizontal)
					+ SScrollBox::Slot().FillContentSize(1.0f)
				[
					TableView.ToSharedRef()
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				VerticalScrollbar
			]
		];
	}

	SStructChooserTableWidget::~SStructChooserTableWidget()
	{
	}


	bool SStructChooserTableWidget::HasSelection() const
	{
		return ChooserViewModel->HasSelection();
	}

	bool SStructChooserTableWidget::HasRowsSelected() const
	{
		return ChooserViewModel->HasRowsSelected();
	}

	bool SStructChooserTableWidget::HasColumnSelected() const
	{
		return ChooserViewModel->HasColumnSelected();
	}

	void SStructChooserTableWidget::SelectRootProperties()
	{
		ChooserViewModel->SelectRootProperties();
	}

	bool SStructChooserTableWidget::CanAutoPopulateSelection() const 
	{
		if (HasColumnSelected())
		{
			return ChooserViewModel->CanAutoPopulateColumn(GetSelectedColumn());
		}
		else if (HasRowsSelected())
		{
			return ChooserViewModel->CanAutoPopulateRows();	
		}
		return false;
	}

	void SStructChooserTableWidget::AutoPopulateSelection()
	{
		if (HasColumnSelected())
		{
			ChooserViewModel->AutoPopulateColumn(GetSelectedColumn());
		}
		else if (HasRowsSelected())
		{
			ChooserViewModel->AutoPopulateRows(ChooserViewModel->GetSelectedRows());
		}
	}

	void SStructChooserTableWidget::AutoPopulateAll()
	{
		ChooserViewModel->AutoPopulateAll();
	}
	
	void SStructChooserTableWidget::RemoveDisabledData()
	{
		ChooserViewModel->RemoveDisabledData();
	}


	void SStructChooserTableWidget::DeleteSelection()
	{
		if (HasColumnSelected())
		{
			ChooserViewModel->DeleteColumn(GetSelectedColumn());
			SelectRootProperties();
		}
		else if (HasRowsSelected())
		{
			ChooserViewModel->DeleteRows(ChooserViewModel->GetSelectedRows());
		}
	}

	void SStructChooserTableWidget::DuplicateSelection()
	{
		if (HasRowsSelected())
		{
			ChooserViewModel->DuplicateRows(ChooserViewModel->GetSelectedRows());
		}
		else if(HasColumnSelected())
		{
			ChooserViewModel->DuplicateColumn(GetSelectedColumn());
		}
	}
	
	bool SStructChooserTableWidget::IsSelectionDisabled() const
	{
		if (HasRowsSelected())
		{
			return ChooserViewModel->AreRowsDisabled(ChooserViewModel->GetSelectedRows());
		}
		else if(HasColumnSelected())
		{
			return ChooserViewModel->IsColumnDisabled(GetSelectedColumn());
		}
		return false;
	}

	void SStructChooserTableWidget::ToggleDisableSelection()
	{
		if (HasRowsSelected())
		{
			ChooserViewModel->ToggleDisableRows(ChooserViewModel->GetSelectedRows());
		}
		else if(HasColumnSelected())
		{
			ChooserViewModel->ToggleDisableColumn(GetSelectedColumn());
		}
	}
	
	void SStructChooserTableWidget::CopySelection()
	{
		if (HasRowsSelected())
		{
			ChooserViewModel->CopyRows(ChooserViewModel->GetSelectedRows());
		}
		else if(HasColumnSelected())
		{
			ChooserViewModel->CopyColumn(GetSelectedColumn());
		}
	}

	void SStructChooserTableWidget::CutSelection()
	{
		if (HasRowsSelected())
		{
			TConstArrayView<int32> SelectedRows = ChooserViewModel->GetSelectedRows();
			ChooserViewModel->CopyRows(SelectedRows);
			ChooserViewModel->DeleteRows(SelectedRows);
		}
		else if(HasColumnSelected())
		{
			int ColumnIndex = GetSelectedColumn();
			ChooserViewModel->CopyColumn(ColumnIndex);
			ChooserViewModel->DeleteColumn(ColumnIndex);
		}
	}

	bool SStructChooserTableWidget::CanPaste() const
	{
		return ChooserViewModel->CanPaste();
	}

	void SStructChooserTableWidget::Paste()
	{
		ChooserViewModel->Paste();
	}

	
	bool SStructChooserTableWidget::CanMoveRowsUp()
	{
		return ChooserViewModel->CanMoveRowsUp(ChooserViewModel->GetSelectedRows());
	}

	void SStructChooserTableWidget::MoveRowsUp()
	{
		if (HasRowsSelected())
		{
			TConstArrayView<int32> SelectedRows = ChooserViewModel->GetSelectedRows();

			int MinSelectedRow = ChooserViewModel->GetNumRows();
			for(int SelectedRow : SelectedRows)
			{
				if (SelectedRow != ColumnWidget_SpecialIndex_Fallback)
				{
					MinSelectedRow = FMath::Min(SelectedRow, MinSelectedRow);
				}
			}
			ChooserViewModel->MoveRows(SelectedRows, MinSelectedRow - 1);
		}
	}

	bool SStructChooserTableWidget::CanMoveRowsDown()
	{
		return ChooserViewModel->CanMoveRowsDown(ChooserViewModel->GetSelectedRows());
	}

	void SStructChooserTableWidget::MoveRowsDown()
	{
		if (HasRowsSelected())
		{
			TConstArrayView<int32> SelectedRows =	ChooserViewModel->GetSelectedRows();

			int MaxSelectedRow = -1;
			for(int SelectedRow : SelectedRows)
			{
				MaxSelectedRow = FMath::Max(SelectedRow, MaxSelectedRow);
			}
			ChooserViewModel->MoveRows(SelectedRows, MaxSelectedRow + 2);
		}
	}

	bool SStructChooserTableWidget::CanMoveColumnLeft()
	{
		return ChooserViewModel->CanMoveColumnLeft(GetSelectedColumn());
	}

	void SStructChooserTableWidget::MoveColumnLeft()
	{
		ChooserViewModel->MoveColumnLeft(GetSelectedColumn());
	}

	bool SStructChooserTableWidget::CanMoveColumnRight()
	{
		return ChooserViewModel->CanMoveColumnRight(GetSelectedColumn());
	}

	void SStructChooserTableWidget::MoveColumnRight()
	{
		ChooserViewModel->MoveColumnRight(GetSelectedColumn());
	}

	void SStructChooserTableWidget::BindCommands(TSharedPtr<FUICommandList> InCommandList)
	{
		CommandList = InCommandList;

		const FChooserTableEditorCommands& Commands = FChooserTableEditorCommands::Get();

		CommandList->MapAction(
			Commands.EditChooserSettings,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::SelectRootProperties));
		
		CommandList->MapAction(
			Commands.AutoPopulateAll,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::AutoPopulateAll));
		
		CommandList->MapAction(
			Commands.RemoveDisabledData,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::RemoveDisabledData));
		
		CommandList->MapAction(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::DeleteSelection),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection)
			);

		CommandList->MapAction(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::DuplicateSelection),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection)
			);

		CommandList->MapAction(
			Commands.AutoPopulateSelection,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::AutoPopulateSelection),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CanAutoPopulateSelection),
			FGetActionCheckState(),FIsActionButtonVisible::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection)
			);
		
		CommandList->MapAction(
			Commands.Disable,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::ToggleDisableSelection),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection),
			FIsActionChecked::CreateSP(SharedThis(this), &SStructChooserTableWidget::IsSelectionDisabled),
			FIsActionButtonVisible::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection)
			);

		CommandList->MapAction(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CopySelection),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection));
		
		CommandList->MapAction(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CutSelection),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasSelection));
		
		CommandList->MapAction(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::Paste),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CanPaste));
		
		CommandList->MapAction(
			Commands.MoveUp,
			FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::MoveRowsUp),
			FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CanMoveRowsUp),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasRowsSelected)
			);
		
		CommandList->MapAction(
				Commands.MoveDown,
				FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::MoveRowsDown),
				FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CanMoveRowsDown),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasRowsSelected)
				);
		
		CommandList->MapAction(
				Commands.MoveLeft,
				FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::MoveColumnLeft),
				FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CanMoveColumnLeft),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasColumnSelected)
				);

		CommandList->MapAction(
				Commands.MoveRight,
				FExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::MoveColumnRight),
				FCanExecuteAction::CreateSP(SharedThis(this), &SStructChooserTableWidget::CanMoveColumnRight),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateSP(SharedThis(this), &SStructChooserTableWidget::HasColumnSelected)
				);
	}

}

#undef LOCTEXT_NAMESPACE
