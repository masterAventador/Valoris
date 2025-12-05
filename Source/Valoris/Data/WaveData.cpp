// Copyright Valoris. All Rights Reserved.

#include "WaveData.h"

const FWaveInfo& UWaveData::GetWaveInfo(int32 WaveIndex) const
{
	check(Waves.IsValidIndex(WaveIndex));
	return Waves[WaveIndex];
}
