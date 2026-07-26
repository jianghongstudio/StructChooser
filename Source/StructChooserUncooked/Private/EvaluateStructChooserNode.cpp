#include "EvaluateStructChooserNode.h"
#include "StructChooserFunctionLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EvaluateStructChooserNode)

#define LOCTEXT_NAMESPACE "EvaluateStructChooserNode"

UK2Node_EvaluateStructChooser::UK2Node_EvaluateStructChooser()
{
}

void UK2Node_EvaluateStructChooser::UnregisterCallbacks()
{
#if WITH_EDITOR
	if (CurrentCallbackChooser)
	{
		CurrentCallbackChooser->OnOutputStructTypeChanged.RemoveAll(this);
		CurrentCallbackChooser->OnContextClassChanged.RemoveAll(this);
		CurrentCallbackChooser = nullptr;
	}
#endif
}

void UK2Node_EvaluateStructChooser::BeginDestroy()
{
	UnregisterCallbacks();
	Super::BeginDestroy();
}

void UK2Node_EvaluateStructChooser::DestroyNode()
{
	UnregisterCallbacks();
	Super::DestroyNode();
}

void UK2Node_EvaluateStructChooser::ChooserChanged()
{
	UnregisterCallbacks();

#if WITH_EDITOR
	if (Chooser)
	{
		Chooser->OnOutputStructTypeChanged.AddUObject(this, &UK2Node_EvaluateStructChooser::OutputStructTypeChanged);
		Chooser->OnContextClassChanged.AddUObject(this, &UK2Node::ReconstructNode);
		CurrentCallbackChooser = Chooser;
	}
#endif

	ReconstructNode();
}

void UK2Node_EvaluateStructChooser::OutputStructTypeChanged(const UScriptStruct*)
{
	ReconstructNode();
}

void UK2Node_EvaluateStructChooser::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropName == GET_MEMBER_NAME_CHECKED(UK2Node_EvaluateStructChooser, Chooser))
	{
		ChooserChanged();
	}
	else if (PropName == GET_MEMBER_NAME_CHECKED(UK2Node_EvaluateStructChooser, Mode))
	{
		ReconstructNode();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UK2Node_EvaluateStructChooser::PostLoad()
{
	Super::PostLoad();
	if (Chooser)
	{
#if WITH_EDITOR
		Chooser->OnOutputStructTypeChanged.AddUObject(this, &UK2Node_EvaluateStructChooser::OutputStructTypeChanged);
		Chooser->OnContextClassChanged.AddUObject(this, &UK2Node::ReconstructNode);
		CurrentCallbackChooser = Chooser;
#endif
	}
}

void UK2Node_EvaluateStructChooser::PreloadRequiredAssets()
{
	if (Chooser)
	{
		Chooser->ConditionalPreload();
	}
	Super::PreloadRequiredAssets();
}

void UK2Node_EvaluateStructChooser::RefreshResultPin()
{
	UEdGraphPin* ResultPin = FindPin(TEXT("Result"), EGPD_Output);
	UScriptStruct* StructType = Chooser ? Chooser->OutputStructType.Get() : nullptr;

	if (!StructType)
	{
		if (ResultPin)
		{
			RemovePin(ResultPin);
		}
		return;
	}

	const EPinContainerType Container = (Mode == EEvaluateStructChooserMode::AllResults)
		? EPinContainerType::Array
		: EPinContainerType::None;

	if (ResultPin)
	{
		ResultPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		ResultPin->PinType.PinSubCategoryObject = StructType;
		ResultPin->PinType.ContainerType = Container;
	}
	else
	{
		UEdGraphNode::FCreatePinParams PinParams;
		PinParams.ContainerType = Container;
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, StructType, TEXT("Result"), PinParams);
	}
}

void UK2Node_EvaluateStructChooser::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	if (!FindPin(UEdGraphSchema_K2::PN_Execute, EGPD_Input))
	{
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	}
	if (!FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output))
	{
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	}

	// Context object (first context param convenience)
	if (!FindPin(TEXT("ContextObject"), EGPD_Input))
	{
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), TEXT("ContextObject"));
	}

	RefreshResultPin();
}

FText UK2Node_EvaluateStructChooser::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (Chooser)
	{
		return FText::Format(LOCTEXT("TitleWithChooser", "Evaluate Struct Chooser: {0}"), FText::FromString(Chooser->GetName()));
	}
	return LOCTEXT("Title", "Evaluate Struct Chooser");
}

FText UK2Node_EvaluateStructChooser::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Evaluates a StructChooserTable and returns the selected struct instance(s). Do not use the engine Evaluate Chooser node with StructChooser assets.");
}

FText UK2Node_EvaluateStructChooser::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Struct Chooser");
}

void UK2Node_EvaluateStructChooser::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

void UK2Node_EvaluateStructChooser::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

void UK2Node_EvaluateStructChooser::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* ExecInput = GetExecPin();
	UEdGraphPin* ExecOutput = GetPassThroughPin(ExecInput);
	UEdGraphPin* ResultPin = FindPin(TEXT("Result"), EGPD_Output);
	UEdGraphPin* ContextObjectPin = FindPin(TEXT("ContextObject"), EGPD_Input);

	if (!ExecInput || !ExecInput->HasAnyConnections())
	{
		BreakAllNodeLinks();
		return;
	}

	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	if (Mode == EEvaluateStructChooserMode::AllResults)
	{
		CallFunction->SetFromFunction(UStructChooserFunctionLibrary::StaticClass()->FindFunctionByName(
			GET_MEMBER_NAME_CHECKED(UStructChooserFunctionLibrary, EvaluateStructChooserTypedMulti)));
	}
	else
	{
		CallFunction->SetFromFunction(UStructChooserFunctionLibrary::StaticClass()->FindFunctionByName(
			GET_MEMBER_NAME_CHECKED(UStructChooserFunctionLibrary, EvaluateStructChooserTyped)));
	}
	CallFunction->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*ExecInput, *CallFunction->GetExecPin());
	if (ExecOutput)
	{
		CompilerContext.MovePinLinksToIntermediate(*ExecOutput, *CallFunction->GetThenPin());
	}

	if (UEdGraphPin* ChooserPin = CallFunction->FindPin(TEXT("ChooserTable")))
	{
		ChooserPin->DefaultObject = Chooser;
	}

	if (ContextObjectPin && CallFunction->FindPin(TEXT("ContextObject")))
	{
		CompilerContext.MovePinLinksToIntermediate(*ContextObjectPin, *CallFunction->FindPin(TEXT("ContextObject")));
	}

	if (ResultPin)
	{
		UEdGraphPin* OutPin = CallFunction->FindPin(Mode == EEvaluateStructChooserMode::AllResults ? TEXT("OutResults") : TEXT("OutResult"));
		if (OutPin)
		{
			OutPin->PinType = ResultPin->PinType;
			CompilerContext.MovePinLinksToIntermediate(*ResultPin, *OutPin);
		}
	}

	BreakAllNodeLinks();
}

#undef LOCTEXT_NAMESPACE
