// Copyright Epic Games, Inc. All Rights Reserved.
//
// UE 5.7 shim — UE5.8 ChooserEditor exposes IChooserTableView; we mirror the minimal surface.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SCompoundWidget.h"

namespace UE::ChooserEditor
{
	class IChooserTableViewModel;

	class IChooserTableView : public SCompoundWidget
	{
	public:
		virtual ~IChooserTableView() override = default;
		virtual TSharedPtr<IChooserTableViewModel> GetViewModel() = 0;
	};
}
