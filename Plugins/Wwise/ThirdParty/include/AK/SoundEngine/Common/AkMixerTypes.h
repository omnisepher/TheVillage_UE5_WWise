/***********************************************************************
  The content of this file includes source code for the sound engine
  portion of the AUDIOKINETIC Wwise Technology and constitutes "Level
  Two Source Code" as defined in the Source Code Addendum attached
  with this file.  Any use of the Level Two Source Code shall be
  subject to the terms and conditions outlined in the Source Code
  Addendum and the End User License Agreement for Wwise(R).

  Copyright (c) 2024 Audiokinetic Inc.
 ***********************************************************************/

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AK {

 /// Coefficients to be used for application of digital biquad filters
struct AkBiquadCoefficients
{
	AkBiquadCoefficients()
		: fB0(0.f)
		, fB1(0.f)
		, fB2(0.f)
		, fA1(0.f)
		, fA2(0.f)
	{}
	AkBiquadCoefficients(AkReal32 _fB0, AkReal32 _fB1, AkReal32 _fB2, AkReal32 _fA1, AkReal32 _fA2)
		: fB0(_fB0)
		, fB1(_fB1)
		, fB2(_fB2)
		, fA1(_fA1)
		, fA2(_fA2)
	{}

	AkReal32 fB0;
	AkReal32 fB1;
	AkReal32 fB2;
	AkReal32 fA1;
	AkReal32 fA2;
};

/// "Memories" storing the previous state of the digital biquad filter
struct AkBiquadMemories
{
	AkReal32 fFFwd2;
	AkReal32 fFFwd1;
	AkReal32 fFFbk2;
	AkReal32 fFFbk1;

	AkBiquadMemories()
	{
		fFFwd2 = 0.f;
		fFFwd1 = 0.f;
		fFFbk2 = 0.f;
		fFFbk1 = 0.f;
	}
};

} // namespace