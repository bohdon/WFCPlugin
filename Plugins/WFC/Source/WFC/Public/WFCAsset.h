// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileSetConfig.h"
#include "Engine/DataAsset.h"
#include "WFCAsset.generated.h"

class UWFCCellSelector;
class UWFCConstraint;
class UWFCGenerator;
class UWFCGeneratorSnapshot;
class UWFCGridConfig;
class UWFCModel;
class UWFCTileSet;


/**
 * A data asset used to define the tiles and classes for use in a WFC generation.
 */
UCLASS(BlueprintType, Blueprintable)
class WFC_API UWFCAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UWFCAsset();

	/** The generator class to use, which handles applying all constraints and running cell and tile selection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, DisplayName = "Generator", Category = "Config")
	TSubclassOf<UWFCGenerator> GeneratorClass;

	/** The constraints to apply, in order of priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Constraints", Category = "Config")
	TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses;

	/** The cell selectors to use, in order of priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Cell Selectors", Category = "Config")
	TArray<TSubclassOf<UWFCCellSelector>> CellSelectorClasses;

	/**
	 * The model class to use, which will generate all tiles and map them to tile ids.
	 * The model is also able to configure the generator and all constraints as desired.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Model", Category = "Config")
	TSubclassOf<UWFCModel> ModelClass;

	/** The grid and configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Config")
	TObjectPtr<UWFCGridConfig> GridConfig;

	/** Only tiles matching this tag query will be used. If empty, all tiles will be used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
	FGameplayTagQuery TileTagQuery;

	/** The tile sets to use, which contain all available tiles as well as relevant data specific to each set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
	TArray<TObjectPtr<UWFCTileSet>> TileSets;

	/** Tile set configs used to define properties for all tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Tiles")
	TArray<TObjectPtr<UWFCTileSetConfig>> TileConfigs;

	/** Return a config by class, or nullptr if it does not exist. */
	template <class T>
	T* GetTileConfig() const
	{
		for (UWFCTileSetConfig* Config : TileConfigs)
		{
			if (Config->IsA<T>())
			{
				return Cast<T>(Config);
			}
		}
		return nullptr;
	}

	/**
	 * Snapshot of the generator after initialization and startup of deterministic constraints.
	 * Initial constraint application is often slow and can be cached in this asset for faster startup.
	 */
	UPROPERTY(EditAnywhere, Instanced, Category = "Snapshot")
	TObjectPtr<UWFCGeneratorSnapshot> StartupSnapshot;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;

	/** Update the startup snapshot to cache the WFC state after deterministic constraints have run the first time. */
	UFUNCTION(BlueprintCallable, CallInEditor)
	virtual void UpdateSnapshot();
#endif
};
