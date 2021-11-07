// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset.h"


UWFCTile2dAsset::UWFCTile2dAsset()
{
	EdgePatterns = {
		{EWFCTile2dEdge::XPos, -1},
		{EWFCTile2dEdge::XNeg, -1},
		{EWFCTile2dEdge::YPos, -1},
		{EWFCTile2dEdge::YNeg, -1},
	};
}
