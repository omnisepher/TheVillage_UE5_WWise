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
	StringMatching.h:
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Array2D.h"

// Longest Common Substring
namespace LCS
{
	struct StringComparer
	{
		StringComparer(FString in_a, FString in_b)
		{
			a = in_a;
			b = in_b;
		}

		void lcs()
		{
			int32 m = a.Len();
			int32 n = b.Len();

			Array2D<int32> suffixLengths(m + 1, n + 1, 0);
			int32 result = 0;
			int32 aIdx = 0;
			int32 bIdx = 0;

			for (int32 i = 0; i <= m; i++)
			{
				for (int32 j = 0; j <= n; j++)
				{
					if (i == 0 || j == 0)
						suffixLengths(i, j) = 0;
					else if (a[i - 1] == b[j - 1])
					{
						suffixLengths(i, j) = suffixLengths(i - 1, j - 1) + 1;
						if (suffixLengths(i, j) > result)
						{
							result = suffixLengths(i, j);
							aIdx = i;
							bIdx = j;
						}
					}
					else
						suffixLengths(i, j) = 0;
				}
			}

			if (result > 2)
			{
				a.RemoveAt(aIdx - result, result);
				b.RemoveAt(bIdx - result, result);

				lcs();
			}
		}

		FString a;
		FString b;
	};

	// Obtain the length of the longest common sub-string between two strings
	// https://en.wikipedia.org/wiki/Longest_common_substring_problem
	float GetLCSScore(const FString& a, const FString& b);
};
