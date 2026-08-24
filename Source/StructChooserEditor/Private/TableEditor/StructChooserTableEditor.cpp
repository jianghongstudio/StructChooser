// Copyright Epic Games, Inc. All Rights Reserved.

#include "StructChooserTableEditor.h"

#include "Chooser.h"
#include "StructChooserEditorDetails.h"
#include "StructChooserRowDetails.h"
#include "StructChooserFindProperties.h"
#include "ChooserTableEditorCommands.h"
#include "ClassViewerFilter.h"
#include "DetailCategoryBuilder.h"
#include "Factories.h"
#include "GraphEditorSettings.h"
#include "IPropertyAccessEditor.h"
#include "LandscapeRender.h"
#include "ObjectChooserClassFilter.h"
#include "ObjectChooserWidgetFactories.h"
#include "ObjectChooser_Asset.h"
#include "ObjectChooser_Class.h"
#include "PersonaModule.h"
#include "StructUtils/PropertyBag.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "RandomizeColumn.h"
#include "SAssetDropTarget.h"
#include "SStructChooserColumnHandle.h"
#include "SClassViewer.h"
#include "ScopedTransaction.h"
#include "SStructNestedChooserTree.h"
#include "SourceCodeNavigation.h"
#include "StructViewerModule.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/StringOutputDevice.h"
#include "Misc/TransactionObjectEvent.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "SStructChooserTableRow.h"
#include "Elements/Framework/TypedElementQueryBuilder.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Views/SListView.h"
#include "ToolMenus.h"
#include "UnrealExporter.h"
#include "Exporters/Exporter.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Framework/Commands/GenericCommands.h"
#include "SPositiveActionButton.h"
#include "SStructChooserTableWidget.h"

#define LOCTEXT_NAMESPACE "ChooserEditor"

namespace UE::StructChooserEditor
{
	
constexpr int32 HistorySize = 16;	

const FName FStructChooserTableEditor::ToolkitFName( TEXT( "StructChooserTableEditor" ) );
const FName FStructChooserTableEditor::PropertiesTabId( TEXT( "StructChooserEditor_Properties" ) );
const FName FStructChooserTableEditor::FindReplaceTabId( TEXT( "StructChooserEditor_FindReplace" ) );
const FName FStructChooserTableEditor::TableTabId( TEXT( "StructChooserEditor_Table" ) );
const FName FStructChooserTableEditor::NestedTablesTreeTabId( TEXT( "StructChooserEditor_NestedTables" ) );

void FStructChooserTableEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_ChooserTableEditor", "Chooser Table Editor"));

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner( PropertiesTabId, FOnSpawnTab::CreateSP(this, &FStructChooserTableEditor::SpawnPropertiesTab) )
		.SetDisplayName( LOCTEXT("PropertiesTab", "Details") )
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon("EditorStyle", "LevelEditor.Tabs.Details"));
		
	InTabManager->RegisterTabSpawner( TableTabId, FOnSpawnTab::CreateSP(this, &FStructChooserTableEditor::SpawnTableTab) )
		.SetDisplayName( LOCTEXT("TableTab", "Chooser Table") )
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon("ChooserEditorStyle", "ChooserEditor.ChooserTableIconSmall"));
		
	InTabManager->RegisterTabSpawner( NestedTablesTreeTabId, FOnSpawnTab::CreateSP(this, &FStructChooserTableEditor::SpawnNestedTablesTreeTab) )
		.SetDisplayName( LOCTEXT("NestedTablesTab", "Nested Choosers") )
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon("ChooserEditorStyle", "ChooserEditor.ChooserTableIconSmall"));


	InTabManager->RegisterTabSpawner( FindReplaceTabId, FOnSpawnTab::CreateSP(this, &FStructChooserTableEditor::SpawnFindReplaceTab) )
		.SetDisplayName( LOCTEXT("FindReplaceTab", "Find/Replace") )
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Find"));
}
	
void FStructChooserTableEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner( TableTabId );
	InTabManager->UnregisterTabSpawner( PropertiesTabId );
	InTabManager->UnregisterTabSpawner( FindReplaceTabId );
}

const FName FStructChooserTableEditor::StructChooserTableToolbarName(TEXT("StructChooserTableToolbar"));
const FName FStructChooserTableEditor::StructChooserEditorAppIdentifier(TEXT("StructChooserEditorApp"));

FStructChooserTableEditor::FStructChooserTableEditor()
{
}

FStructChooserTableEditor::~FStructChooserTableEditor()
{
	FCoreUObjectDelegates::OnObjectsReplaced.RemoveAll(this);
}

FName FStructChooserTableEditor::EditorName = "StructChooserTableEditor";
	
FName FStructChooserTableEditor::ContextMenuName("StructChooserEditorContextMenu");
	
FName FStructChooserTableEditor::GetEditorName() const
{
	return EditorName;
}



UChooserTable* FStructChooserTableEditor::GetRootChooser()
{
	return ViewModel->GetRootChooser();
}

void FStructChooserTableEditor::RegisterMenus()
{
	ViewModel->RegisterMenus(GetToolkitCommands());
	
	struct Local
	{
		static void FillEditMenu(FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("ChooserEditing", LOCTEXT("Chooser Table Editing", "Chooser Table"));
			{
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy, NAME_None);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut, NAME_None);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste, NAME_None);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate, NAME_None, LOCTEXT("Duplicate Selection", "Duplicate Selection"));
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete, NAME_None, LOCTEXT("Delete Selection", "Delete Selection"));
				MenuBuilder.AddMenuEntry(FChooserTableEditorCommands::Get().Disable, NAME_None, LOCTEXT("Disable Selection", "Disable Selection"));
				MenuBuilder.AddMenuEntry(FChooserTableEditorCommands::Get().RemoveDisabledData, NAME_None);
			}
			MenuBuilder.EndSection();
		}
	};
	
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

	// Extend the Edit menu
	MenuExtender->AddMenuExtension(
		"EditHistory",
		EExtensionHook::After,
		GetToolkitCommands(),
		FMenuExtensionDelegate::CreateStatic(&Local::FillEditMenu));

	AddMenuExtender(MenuExtender);
}

void FStructChooserTableEditor::InitToolMenuContext(FToolMenuContext& MenuContext)
{
	FAssetEditorToolkit::InitToolMenuContext(MenuContext);
	UStructChooserEditorToolMenuContext* Context = NewObject<UStructChooserEditorToolMenuContext>();
	Context->ViewModel = ViewModel;
	MenuContext.AddObject(Context);
	
	MenuContext.AppendCommandList(ToolkitCommands);
}
	
void FStructChooserTableEditor::SaveAsset_Execute()
{
	ViewModel->AutoPopulateAll();
	FAssetEditorToolkit::SaveAsset_Execute();
}

void FStructChooserTableEditor::InitEditor( const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, const TArray<UObject*>& ObjectsToEdit, FGetDetailsViewObjects GetDetailsViewObjects )
{
	UChooserTable* Chooser = Cast<UChooserTable>(ObjectsToEdit[0]);
	
	ViewModel = MakeShared<FStructChooserTableViewModel>(Chooser);
	ViewModel->SetOpenObjectDelegate(FOpenObject::CreateLambda([this](UObject* Object)
	{
		if (UChooserTable* Chooser = Cast<UChooserTable>(Object))
		{
			SetChooserTableToEdit(Chooser);
		}
	}));

	History.Reserve(HistorySize);
	
	BreadcrumbTrail = SNew(SBreadcrumbTrail<UChooserTable*>)
		.ButtonStyle(FAppStyle::Get(), "GraphBreadcrumbButton")
		.TextStyle(FAppStyle::Get(), "GraphBreadcrumbButtonText")
		.ButtonContentPadding( FMargin(4.f, 2.f) )
		.DelimiterImage( FAppStyle::GetBrush("BreadcrumbTrail.Delimiter") )
		.OnCrumbPushed_Lambda([this](UChooserTable* Table)
		{
			ViewModel->SetChooser(BreadcrumbTrail->PeekCrumb());
		})
		.OnCrumbClicked_Lambda([this](UChooserTable* Table)
		{
			AddHistory();
			ViewModel->SetChooser(BreadcrumbTrail->PeekCrumb());
		})
		.GetCrumbMenuContent_Lambda([this](UChooserTable* Item)
		{
			return MakeChoosersMenu(Item);
		})
	;

	BreadcrumbTrail->PushCrumb(FText::FromString(Chooser->GetName()), Chooser);
	AddHistory();

	FCoreUObjectDelegates::OnObjectsReplaced.AddSP(this, &FStructChooserTableEditor::OnObjectsReplaced);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NotifyHook = ViewModel.Get();
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView( DetailsViewArgs );

	ViewModel->SetShowDetailsDelegate(FShowDetails::CreateLambda([DetailsViewWeakPtr = DetailsView.ToWeakPtr()](const TArray<UObject*>& Objects)
		{
			if (TSharedPtr<IDetailsView> Details = DetailsViewWeakPtr.Pin())
			{
				Details->SetObjects(Objects, true);
			}
		}));
	
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout( "Standalone_StructChooserTableEditor_Layout_v1.6" )
	->AddArea
	(
		FTabManager::NewPrimaryArea() ->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.7f)
				->AddTab( TableTabId, ETabState::OpenedTab )
			)
			->Split
			(
			FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.3f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->AddTab( PropertiesTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->AddTab( NestedTablesTreeTabId, ETabState::OpenedTab )
				)
			)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor( Mode, InitToolkitHost, FStructChooserTableEditor::StructChooserEditorAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectsToEdit );

	ViewModel->RegisterMenus(GetToolkitCommands());
	
	RegenerateMenusAndToolbars();

	ViewModel->SelectRootProperties();
	SetChooserTableToEdit(Chooser);
		
	FAnimAssetFindReplaceConfig FindReplaceConfig;
	FindReplaceConfig.InitialProcessorClass = UStructChooserFindProperties::StaticClass();
}

void FStructChooserTableEditor::FocusWindow(UObject* ObjectToFocusOn)
{
	if (UChooserTable* Chooser = Cast<UChooserTable>(ObjectToFocusOn))
	{
		SetChooserTableToEdit(Chooser);
	}
	FAssetEditorToolkit::FocusWindow(ObjectToFocusOn);
}

FName FStructChooserTableEditor::GetToolkitFName() const
{
	return ToolkitFName;
}

FText FStructChooserTableEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Chooser Table Editor");
}

void FStructChooserTableEditor::RefreshNestedObjectTree()
{
	if (NestedChooserTree.IsValid())
	{
		NestedChooserTree->RefreshAll();
	}
}

FText FStructChooserTableEditor::GetToolkitName() const
{
	check( ViewModel->GetRootChooser() );
	return FText::FromString(ViewModel->GetRootChooser()->GetName());
}

FText FStructChooserTableEditor::GetToolkitToolTipText() const
{
	check( ViewModel->GetRootChooser() );
	return FAssetEditorToolkit::GetToolTipTextForObject(ViewModel->GetRootChooser());
}

FLinearColor FStructChooserTableEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor( 0.5f, 0.0f, 0.0f, 0.5f );
}

void FStructChooserTableEditor::SetPropertyVisibilityDelegate(FIsPropertyVisible InVisibilityDelegate)
{
	DetailsView->SetIsPropertyVisibleDelegate(InVisibilityDelegate);
	DetailsView->ForceRefresh();
}

void FStructChooserTableEditor::SetPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled InPropertyEditingDelegate)
{
	DetailsView->SetIsPropertyEditingEnabledDelegate(InPropertyEditingDelegate);
	DetailsView->ForceRefresh();
}


TSharedRef<SDockTab> FStructChooserTableEditor::SpawnPropertiesTab( const FSpawnTabArgs& Args )
{
	check( Args.GetTabId() == PropertiesTabId );

	return SNew(SDockTab)
		.Label( LOCTEXT("GenericDetailsTitle", "Details") )
		.TabColorScale( GetTabColorScale() )
		.OnCanCloseTab_Lambda([]() { return false; })
		[
			DetailsView.ToSharedRef()
		];
}
	
TSharedRef<SDockTab> FStructChooserTableEditor::SpawnFindReplaceTab( const FSpawnTabArgs& Args )
{
	check( Args.GetTabId() == FindReplaceTabId );

	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
	FAnimAssetFindReplaceConfig Config;
	Config.InitialProcessorClass = UStructChooserFindProperties::StaticClass();
	return SNew(SDockTab)
		.Label( LOCTEXT("FindReplaceTitle", "Find/Replace") )
		.TabColorScale( GetTabColorScale() )
	[
		PersonaModule.CreateFindReplaceWidget(Config)
	];
}
	
TSharedRef<SDockTab> FStructChooserTableEditor::SpawnNestedTablesTreeTab( const FSpawnTabArgs& Args )
{
	check( Args.GetTabId() == NestedTablesTreeTabId );

	return SNew(SDockTab)
		.Label( LOCTEXT("NestedChooserTreeTitle", "Nested Choosers") )
		[
			SAssignNew(NestedChooserTree, SStructNestedChooserTree).ChooserEditor(this)
		];
}

TSharedRef<SDockTab> FStructChooserTableEditor::SpawnTableTab( const FSpawnTabArgs& Args )
{
	check( Args.GetTabId() == TableTabId );

	UChooserTable* Chooser = ViewModel->GetChooser();

	TSharedRef<SWidget> ChooserTableView = SNew(SStructChooserTableWidget)
											.Commands(GetToolkitCommands())
											.ViewModel(ViewModel);

	ViewModel->RefreshAll();

	TSharedRef<SComboButton> EditChooserTableButton = SNew(SComboButton)
		.ButtonStyle(FAppStyle::Get(), "GraphBreadcrumbButton");
	
	EditChooserTableButton->SetOnGetMenuContent(
    		FOnGetContent::CreateLambda(
    			[this]()
                		{
							return MakeChoosersMenu(ViewModel->GetRootChooser()->GetPackage());
                		})
    		);

	return SNew(SDockTab)
		.Label( LOCTEXT("ChooserTableTitle", "Chooser Table") )
		.TabColorScale( GetTabColorScale() )
		.OnCanCloseTab_Lambda([]() { return false; })
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(3)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "GraphBreadcrumbButton")
						.IsEnabled(this, &FStructChooserTableEditor::CanNavigateBack)
						.OnClicked_Lambda([this]()
						{
							NavigateBack();
							return FReply::Handled();
						})
						.Content()
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush("Icons.ArrowLeft"))
						]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "GraphBreadcrumbButton")
						.IsEnabled(this, &FStructChooserTableEditor::CanNavigateForward)
						.OnClicked_Lambda([this]()
						{
							NavigateForward();
							return FReply::Handled();
						})
						.Content()
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush("Icons.ArrowRight") )
						]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					EditChooserTableButton
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					BreadcrumbTrail.ToSharedRef()
				]

				
			]
			+ SVerticalBox::Slot().FillHeight(1)
			[
					ChooserTableView
			]
		];
}

void FStructChooserTableEditor::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	bool bChangedAny = false;

	UChooserTable* RootChooser = ViewModel->GetRootChooser();

	UObject* ReplacedObject = ReplacementMap.FindRef(RootChooser);

	if (ReplacedObject && ReplacedObject != RootChooser)
	{
		RootChooser = Cast<UChooserTable>(ReplacedObject);
		SetChooserTableToEdit(RootChooser);
		ViewModel->SelectRootProperties();
	}
}

FString FStructChooserTableEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Chooser Table Asset ").ToString();
}

TSharedRef<FStructChooserTableEditor> FStructChooserTableEditor::CreateEditor( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit, FGetDetailsViewObjects GetDetailsViewObjects )
{
	TSharedRef< FStructChooserTableEditor > NewEditor( new FStructChooserTableEditor() );

	TArray<UObject*> ObjectsToEdit;
	ObjectsToEdit.Add( ObjectToEdit );
	NewEditor->InitEditor( Mode, InitToolkitHost, ObjectsToEdit, GetDetailsViewObjects );

	return NewEditor;
}

TSharedRef<FStructChooserTableEditor> FStructChooserTableEditor::CreateEditor( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UObject*>& ObjectsToEdit, FGetDetailsViewObjects GetDetailsViewObjects )
{
	TSharedRef< FStructChooserTableEditor > NewEditor( new FStructChooserTableEditor() );
	NewEditor->InitEditor( Mode, InitToolkitHost, ObjectsToEdit, GetDetailsViewObjects );
	return NewEditor;
}


void FStructChooserTableEditor::AddHistory()
{
	// remove anything ahead of this in the history, if we had gone back
	while (HistoryIndex !=0)
	{
		History.PopFront();
		HistoryIndex--;
	}
	
	if (History.Num() >= HistorySize)
	{
		History.Pop();
	}
	History.AddFront(ViewModel->GetChooser());
}

bool FStructChooserTableEditor::CanNavigateBack() const
{
	return HistoryIndex < History.Num() - 1;
}

void FStructChooserTableEditor::NavigateBack()
{
	if (HistoryIndex < History.Num() - 1)
	{
		HistoryIndex++;
		SetChooserTableToEdit(History[HistoryIndex], false);
	}
}

bool FStructChooserTableEditor::CanNavigateForward() const
{
	return HistoryIndex > 0;
}

void FStructChooserTableEditor::NavigateForward()
{
	if (HistoryIndex > 0)
	{
		HistoryIndex--;
		SetChooserTableToEdit(History[HistoryIndex], false);
	}
}

void FStructChooserTableEditor::SetChooserTableToEdit(UChooserTable* Chooser, bool bApplyToHistory)
{
	if (!Chooser || !ViewModel.IsValid() || Chooser == ViewModel->GetChooser())
	{
		return;
	}

	TArray<UChooserTable*> OuterList;
	const UChooserTable* RootChooser = ViewModel->GetRootChooser();
	for (UChooserTable* Current = Chooser; Current; Current = Cast<UChooserTable>(Current->GetOuter()))
	{
		OuterList.Push(Current);
		if (Current == RootChooser)
		{
			break;
		}
	}

	if (OuterList.IsEmpty() || OuterList.Last() != RootChooser)
	{
		return;
	}

	BreadcrumbTrail->ClearCrumbs();

	while(!OuterList.IsEmpty())
	{
		UChooserTable* Popped = OuterList.Pop();
		BreadcrumbTrail->PushCrumb(FText::FromString(Popped->GetName()), Popped);
	}
	
	if (bApplyToHistory)
	{
		AddHistory();
	}
	
	ViewModel->SetChooser(Chooser);
}

void FStructChooserTableEditor::PushChooserTableToEdit(UChooserTable* Chooser)
{
	BreadcrumbTrail->PushCrumb(FText::FromString(Chooser->GetName()), Chooser);
	AddHistory();
	ViewModel->SetChooser(Chooser);
}
	
void FStructChooserTableEditor::PopChooserTableToEdit()
{
	if (BreadcrumbTrail->HasCrumbs())
	{
		BreadcrumbTrail->PopCrumb();
		ViewModel->SetChooser(BreadcrumbTrail->PeekCrumb());
	}
}

	
void FStructChooserTableEditor::MakeChoosersMenuRecursive(UObject* Outer, FMenuBuilder& MenuBuilder, const FString& Indent = "") 
{
	TArray<UObject*> ChildObjects;
	GetObjectsWithOuter(Outer, ChildObjects, false);

	FString SubIndent = Indent + "    ";
	for (UObject* Object : ChildObjects)
	{
		if (UChooserTable* Chooser = Cast<UChooserTable>(Object))
		{
			if (Chooser == ViewModel->GetRootChooser() || Chooser->GetRootChooser()->NestedObjects.Contains(Chooser))
			{
				MenuBuilder.AddMenuEntry( FText::FromString(Indent + Chooser->GetName()), LOCTEXT("Edit Chooser ToolTip", "Browse to this Nested Chooser Table"), FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([this, Chooser]()
					{
						SetChooserTableToEdit(Chooser);
					})));

				MakeChoosersMenuRecursive(Chooser, MenuBuilder, SubIndent);
			}
		}
	}
}
	
TSharedRef<SWidget> FStructChooserTableEditor::MakeChoosersMenu(UObject* RootObject)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MakeChoosersMenuRecursive(RootObject, MenuBuilder);

	return MenuBuilder.MakeWidget();
}

	
	
void FStructChooserTableEditor::RegisterWidgets()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		TEXT("StructChooserRowDetails"),
		FOnGetDetailCustomizationInstance::CreateStatic(&FStructChooserRowDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		TEXT("StructChooserColumnDetails"),
		FOnGetDetailCustomizationInstance::CreateStatic(&UE::StructChooserEditor::FStructChooserColumnDetails::MakeInstance));
}

}

#undef LOCTEXT_NAMESPACE

