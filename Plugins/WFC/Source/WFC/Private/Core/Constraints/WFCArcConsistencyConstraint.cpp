// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCArcConsistencyConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Allowed Tile Adds"), STAT_WFCArcConsistencyAdds, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Allowed Tile Entries"), STAT_WFCArcConsistencyEntries, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Arc Consistency - Update Time (ms)"), STAT_WFCArcConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Checks"), STAT_WFCArcConstraintNumChecks, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Bans"), STAT_WFCArcConstraintNumBans, STATGROUP_WFC);


void UWFCArcConstraintSnapshot::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << AllowedTiles;
	Ar << SupportCounts;
	Ar << DefaultSupportCounts;
	Ar << BansToPropagate;
}

UWFCArcConsistencyConstraint::UWFCArcConsistencyConstraint()
	: bIgnoreContradictionCells(false),
	  bIsInitialized(false),
	  bDidApplyInitialConsistency(false)
{
}

void UWFCArcConsistencyConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	SET_DWORD_STAT(STAT_WFCArcConsistencyAdds, 0);
	SET_DWORD_STAT(STAT_WFCArcConsistencyEntries, 0);
	SET_FLOAT_STAT(STAT_WFCArcConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumBans, 0);

	if (bIsInitialized)
	{
		return;
	}

	bDidApplyInitialConsistency = false;
	BansToPropagate.Reset();
	VisitedDuringPropagation.Reset();

	// initialize allowed tiles to empty list for each combination of [tile][direction].
	AllowedTiles.AddZeroed(Model->GetNumTiles());
	for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		AllowedTiles[TileId].AddZeroed(Grid->GetNumDirections());
	}

	// initialize support counts array (but don't fill it out or ban tiles yet)
	SupportCounts.Empty(Grid->GetNumCells());
	SupportCounts.AddZeroed(Grid->GetNumCells());
	for (int32 CellIndex = 0; CellIndex < Grid->GetNumCells(); ++CellIndex)
	{
		SupportCounts[CellIndex].AddZeroed(Model->GetNumTiles());
		for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
		{
			SupportCounts[CellIndex][TileId].AddZeroed(Grid->GetNumDirections());
		}
	}

	// store for quick resetting
	DefaultSupportCounts = SupportCounts;

	bIsInitialized = true;
}

void UWFCArcConsistencyConstraint::Reset()
{
	Super::Reset();

	SET_FLOAT_STAT(STAT_WFCArcConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumBans, 0);

	bDidApplyInitialConsistency = false;
	BansToPropagate.Reset();
	VisitedDuringPropagation.Reset();
	SupportCounts = DefaultSupportCounts;
}

void UWFCArcConsistencyConstraint::AddAllowedTileForDirection(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId)
{
	INC_DWORD_STAT(STAT_WFCArcConsistencyAdds);
	if (!AllowedTiles[TileId][Direction].Contains(AllowedTileId))
	{
		AllowedTiles[TileId][Direction].Add(AllowedTileId);
		INC_DWORD_STAT(STAT_WFCArcConsistencyEntries);
	}
}

const TArray<FWFCTileId>& UWFCArcConsistencyConstraint::GetAllowedTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const
{
	return AllowedTiles[TileId][Direction];
}

UWFCConstraintSnapshot* UWFCArcConsistencyConstraint::CreateSnapshot(UObject* Outer) const
{
	UWFCArcConstraintSnapshot* Snapshot = NewObject<UWFCArcConstraintSnapshot>(Outer);
	Snapshot->AllowedTiles = AllowedTiles;
	Snapshot->SupportCounts = SupportCounts;
	Snapshot->DefaultSupportCounts = DefaultSupportCounts;
	Snapshot->BansToPropagate = BansToPropagate;
	return Snapshot;
}

void UWFCArcConsistencyConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	const UWFCArcConstraintSnapshot* ArcSnapshot = Cast<UWFCArcConstraintSnapshot>(Snapshot);
	if (!ArcSnapshot)
	{
		return;
	}
	AllowedTiles = ArcSnapshot->AllowedTiles;
	SupportCounts = ArcSnapshot->SupportCounts;
	DefaultSupportCounts = ArcSnapshot->DefaultSupportCounts;
	BansToPropagate = ArcSnapshot->BansToPropagate;

	bIsInitialized = true;
	bDidApplyInitialConsistency = true;
}

void UWFCArcConsistencyConstraint::NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
	// update support counts
	for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
	{
		SupportCounts[CellIndex][BannedTileId][Direction] -= 1;
	}

	BansToPropagate.Push(FWFCCellIndexAndTileId(CellIndex, BannedTileId));
}

bool UWFCArcConsistencyConstraint::Next()
{
	STAT(const double StartTime = FPlatformTime::Seconds());
	SET_FLOAT_STAT(STAT_WFCArcConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumBans, 0);

	// check all cells and ban tile candidates to reach consistency
	if (!bDidApplyInitialConsistency)
	{
		ApplyInitialConsistency();
		bDidApplyInitialConsistency = true;
	}

	const bool bDidMakeChanges = PropagateChanges();

	INC_FLOAT_STAT_BY(STAT_WFCArcConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);

	return bDidMakeChanges;
}

void UWFCArcConsistencyConstraint::ApplyInitialConsistency()
{
	// initialize support counts for each [CellIndex][TileId][Direction]
	for (int32 CellIndex = 0; CellIndex < Grid->GetNumCells(); ++CellIndex)
	{
		FWFCCell& Cell = Generator->GetCell(CellIndex);

		for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
		{
			for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
			{
				const int32 NeighborCellIndex = Grid->GetCellIndexInDirection(CellIndex, Direction);
				if (!Grid->IsValidCellIndex(NeighborCellIndex))
				{
					continue;
				}

				// support count is the number of compatible tile ids that exist in a
				// direction from one cell to another, for a specific tile id.
				const int32 SupportCount = AllowedTiles[TileId][Direction].Num();
				SupportCounts[CellIndex][TileId][Direction] = SupportCount;
				if (SupportCount == 0 && Cell.TileCandidates.Contains(TileId))
				{
					if (Generator->Ban(CellIndex, TileId) && !bIgnoreContradictionCells)
					{
						return;
					}
					break;
				}
			}
		}
	}
}

bool UWFCArcConsistencyConstraint::PropagateChanges()
{
#if !UE_BUILD_SHIPPING
	VisitedDuringPropagation.Reset();
#endif

	bool bDidAnyWork = false;
	while (!BansToPropagate.IsEmpty())
	{
		bDidAnyWork = true;
		const FWFCCellIndexAndTileId BanToPropagate = BansToPropagate.Pop();

		// update cells in each direction around the affected cell
		for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
		{
			const FWFCCellIndex NeighborCellIndex = Grid->GetCellIndexInDirection(BanToPropagate.CellIndex, Direction);
			if (!Grid->IsValidCellIndex(NeighborCellIndex))
			{
				continue;
			}

#if !UE_BUILD_SHIPPING
			VisitedDuringPropagation.AddUnique(FWFCCellIndexAndDirection(BanToPropagate.CellIndex, Direction));
#endif

			const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

			// use the outgoing direction from the banned tile to determine which tile id's were supported,
			// then decrease the support count for each one.
			const TArray<FWFCTileId>& SupportedTiles = AllowedTiles[BanToPropagate.TileId][Direction];
			for (const FWFCTileId& SupportedTileId : SupportedTiles)
			{
				// Decrement the support count for the supported tile. 
				// e.g. if tile 1 can have tile 2, 3, or 4 next to it in Direction, it starts with 3 supports.
				// when tile 3 is banned from the neighbor cell, it loses a support, if all are lost then
				// tile 1 is no longer a valid candidate.
				const int32 SupportCount = --SupportCounts[NeighborCellIndex][SupportedTileId][InvDirection];
				if (SupportCount == 0)
				{
					// no more supports left, ban this tile id for the neighbor
					if (Generator->Ban(NeighborCellIndex, SupportedTileId) && !bIgnoreContradictionCells)
					{
						// contradiction
						return true;
					}
				}
			}
		}

		if (Generator->StepGranularity >= EWFCGeneratorStepGranularity::ConstraintDetailed)
		{
			// break after each ban propagation
			break;
		}
	}
	return bDidAnyWork;
}

void UWFCArcConsistencyConstraint::LogDebugInfo() const
{
	Super::LogDebugInfo();

	UE_LOG(LogWFC, Verbose, TEXT("%s AllowedTiles allocated size: %.3fKB"),
	       *GetClass()->GetName(), AllowedTiles.GetAllocatedSize() / 1024.f);
	UE_LOG(LogWFC, Verbose, TEXT("%s SupportCounts allocated size: %.3fKB"),
	       *GetClass()->GetName(), SupportCounts.GetAllocatedSize() / 1024.f);


	if (!Model)
	{
		return;
	}

	for (FWFCTileId TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		const FString TileStr = Model->GetTileDebugString(TileId);
		UE_LOG(LogWFC, VeryVerbose, TEXT("%s allowed tiles:"), *TileStr);

		const auto& AllowedDirections = AllowedTiles[TileId];
		for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
		{
			// log the opposite direction, since allowed tiles are stored as an 'incoming' direction
			// but it makes more sense to read this as the 'direction from this tile'
			const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);
			const FString DirectionStr = Grid->GetDirectionName(InvDirection);

			TArray<FString> TileStrs;
			const auto& ThisAllowedTiles = AllowedDirections[Direction];
			for (const int32& AllowedTileId : ThisAllowedTiles)
			{
				TileStrs.Add(FString::FromInt(AllowedTileId));
			}

			UE_LOG(LogWFC, VeryVerbose, TEXT("    %s: %s"), *DirectionStr, *FString::Join(TileStrs, TEXT(", ")));
		}
	}
}
