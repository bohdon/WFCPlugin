// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCTypes.h"


bool FWFCCell::AddCandidate(FWFCTileId TileId)
{
	if (!TileCandidates.Contains(TileId))
	{
		TileCandidates.Add(TileId);
		return true;
	}
	return false;
}

bool FWFCCell::RemoveCandidate(FWFCTileId TileId)
{
	if (TileCandidates.Contains(TileId))
	{
		TileCandidates.Remove(TileId);
		return true;
	}
	return false;
}

bool FWFCCell::HasAnyMatchingCandidate(const TArray<FWFCTileId>& TileIds) const
{
	return TileIds.ContainsByPredicate([this](const FWFCTileId& TileId)
	{
		return TileCandidates.Contains(TileId);
	});
}
