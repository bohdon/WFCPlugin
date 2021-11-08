// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTypes.h"


void FWFCCell::AddCandidate(FWFCTileId TileId)
{
	TileCandidates.AddUnique(TileId);
}

void FWFCCell::RemoveCandidate(FWFCTileId TileId)
{
	if (TileCandidates.Contains(TileId))
	{
		TileCandidates.Remove(TileId);
	}
}
