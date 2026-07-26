#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "StructChooserTable.h"
#include "EvaluateStructChooserNode.generated.h"

UENUM()
enum class EEvaluateStructChooserMode : uint8
{
	FirstResult,
	AllResults
};

UCLASS()
class UK2Node_EvaluateStructChooser : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_EvaluateStructChooser();

	//~ UObject
	virtual void BeginDestroy() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	virtual void DestroyNode() override;

	//~ UEdGraphNode
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	//~ UK2Node
	virtual bool IsNodePure() const override { return false; }
	virtual bool NodeCausesStructuralBlueprintChange() const override { return true; }
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual void PreloadRequiredAssets() override;

private:
	void ChooserChanged();
	void OutputStructTypeChanged(const UScriptStruct*);
	void UnregisterCallbacks();
	void RefreshResultPin();

	UPROPERTY(EditAnywhere, Category = "Struct Chooser")
	TObjectPtr<UStructChooserTable> Chooser;

	UPROPERTY(EditAnywhere, Category = "Struct Chooser")
	EEvaluateStructChooserMode Mode = EEvaluateStructChooserMode::FirstResult;

	TObjectPtr<UStructChooserTable> CurrentCallbackChooser = nullptr;
};
