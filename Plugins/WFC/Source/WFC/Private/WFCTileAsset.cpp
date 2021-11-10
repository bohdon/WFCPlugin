// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset.h"


UWFCTile2DAsset::UWFCTile2DAsset()
{
	EdgeSocketTypes = {
		{EWFCTile2DEdge::XPos, 0},
		{EWFCTile2DEdge::YPos, 0},
		{EWFCTile2DEdge::XNeg, 0},
		{EWFCTile2DEdge::YNeg, 0},
	};
}

UWFCTile3DAsset::UWFCTile3DAsset()
{
	EdgeSocketTypes = {
		{EWFCTile3DEdge::XPos, 0},
		{EWFCTile3DEdge::YPos, 0},
		{EWFCTile3DEdge::XNeg, 0},
		{EWFCTile3DEdge::YNeg, 0},
		{EWFCTile3DEdge::ZPos, 0},
		{EWFCTile3DEdge::ZNeg, 0},
	};
}
