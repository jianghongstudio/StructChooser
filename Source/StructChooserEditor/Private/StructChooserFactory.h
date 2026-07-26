#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "StructChooserFactory.generated.h"

UCLASS()
class UStructChooserTableFactory : public UFactory
{
	GENERATED_BODY()

public:
	UStructChooserTableFactory();

	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	UPROPERTY()
	TObjectPtr<UScriptStruct> OutputStructType;
};
