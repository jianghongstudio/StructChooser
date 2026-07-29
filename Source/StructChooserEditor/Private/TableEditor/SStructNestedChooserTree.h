// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

class UChooserTable;

namespace UE::StructChooserEditor
{

class FStructChooserTableEditor;

struct FNestedChooserTreeEntry
{
	UObject* Object = nullptr;
	bool bExpanded = true;
};

class SStructNestedChooserTree : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStructNestedChooserTree)
	{}
	SLATE_ARGUMENT(FStructChooserTableEditor*, ChooserEditor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SStructNestedChooserTree() override;

	void RefreshAll();
private:
	TSharedPtr<SWidget> TreeViewContextMenuOpening();
	TSharedRef<ITableRow> TreeViewGenerateRow(TSharedPtr<FNestedChooserTreeEntry> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void TreeViewGetChildren(TSharedPtr<FNestedChooserTreeEntry> InItem, TArray<TSharedPtr<FNestedChooserTreeEntry>>& OutChildren);
	void TreeViewDoubleClicked(TSharedPtr<FNestedChooserTreeEntry> SelectedObject);
	void DeleteNestedObject();
	void RenameNestedObject();
	
	FStructChooserTableEditor* ChooserEditor = nullptr;
	UChooserTable* RootChooser = nullptr;
	TSharedPtr<STreeView<TSharedPtr<FNestedChooserTreeEntry>>> TreeView;

	TArray<TSharedPtr<FNestedChooserTreeEntry>> TreeEntries;
	TArray<TSharedPtr<FNestedChooserTreeEntry>> AllChoosers;
};

}
