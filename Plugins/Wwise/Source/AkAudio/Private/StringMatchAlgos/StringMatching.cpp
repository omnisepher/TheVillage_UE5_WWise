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

/*=============================================================================
	StringMatching.cpp:
=============================================================================*/
#include "StringMatching.h"
#include "Array2D.h"

float LCS::GetLCSScore(const FString& a, const FString& b)
{
	StringComparer compare(a, b);
	compare.lcs();

	float score = (float)(compare.a.Len() + compare.b.Len()) / (a.Len() + b.Len());

	float subscore;
	if (compare.a.Len() < compare.b.Len())
		subscore = (float)compare.a.Len() / a.Len();
	else
		subscore = (float)compare.b.Len() / b.Len();

	score = (1.f - score) * (1.f - subscore);

	return score;
}
