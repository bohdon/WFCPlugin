// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCModel.h"


void UWFCModel::Initialize(const UObject* TileData)
{
	TileDataRef = TileData;
}

void UWFCModel::GenerateTiles()
{
	// implement in subclass
}

void UWFCModel::ConfigureGenerator(UWFCGenerator* Generator)
{
	// implement in subclass
}

FWFCTileId UWFCModel::AddTile(TSharedPtr<FWFCModelTile> Tile)
{
	check(Tiles.Num() == TileWeights.Num());

	Tile->Id = Tiles.Num();
	Tiles.Add(Tile);
	TileWeights.Add(Tile->Weight);

	return Tile->Id;
}

FWFCModelTile* UWFCModel::GetTile(FWFCTileId TileId) const
{
	return Tiles.IsValidIndex(TileId) ? Tiles[TileId].Get() : nullptr;
}
