/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal

License Usage

Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "Widgets/SWwiseReconcile.h"
#include "Widgets/Views/SListView.h"
#include "Wwise/WwiseReconcile.h"


class SWwiseReconcileListView : public SListView< TSharedPtr<FWwiseReconcileItem> >
{
public:

	~SWwiseReconcileListView() {};

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedRef<SWwiseReconcile> Owner);

	/** Weak reference to the picker widget that owns this list */
	TWeakPtr<SWwiseReconcile> WwiseReconcileWeak;
};

/** Widget that represents a row in the picker's tree control.  Generates widgets for each column on demand. */
class SWwiseReconcileRow
	: public SMultiColumnTableRow< TSharedPtr<FWwiseReconcileItem> >
{
public:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnId) override;

	/** The item associated with this row of data */
	TWeakPtr<FWwiseReconcileItem> Item;

	/** Weak reference to the picker widget that owns this list */
	TWeakPtr<SWwiseReconcile> WwiseReconcileWeak;

	SLATE_BEGIN_ARGS(SWwiseReconcileRow) {}

	/** The list item for this row */
	SLATE_ARGUMENT(TSharedPtr < FWwiseReconcileItem >, Item)

	SLATE_END_ARGS()

	/** Construct function for this widget */
	void Construct(const FArguments& InArgs, const TSharedRef<SWwiseReconcileListView>& ReconcileListView, TSharedRef<SWwiseReconcile>
		WwiseReconcile);

};