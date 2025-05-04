# WFC Plugin

An Unreal plugin for procedural generation using [Wave Function Collapse](https://github.com/mxgmn/WaveFunctionCollapse)
algorithms.

> Note: This plugin currently conflicts with the built-in `WaveFunctionCollapse` plugin, which uses a ShortName of
`WFC`. Disable that plugin or change this plugin's module name if both are needed.

## Features

- Game and level generation oriented features.
- Shape agnostic grid (2D, 3D, and others).
- Modular constraint setup.

https://github.com/bohdon/WFCPlugin/assets/338928/0d3f8f9d-149a-4524-adda-46e5446d6873

## Overview

- The `UWFCGenerator` brings together several pieces needed to go from source data to a grid of selected tiles.
    - A `UWFCModel` defines all available tiles.
    - `UWFCGridConfig` specifies which grid class to use, grid dimensions, etc
    - An array of `UWFCConstraint` objects set the rules for how tiles can be placed.
        - _What tiles can be adjacent to each other?_
        - _How many times can a tile be used?_
    - One or more `UWFCCellSelector` objects that handle picking which cell to collapse next.
        - The most common method is based on entropy -- basically the cells that have the last amount of
          possible solutions are collapsed first, since they're likely to cause conflicts if put off until later.
- With those pieces in place, the generator can be run until all cells have selected a tile.
- During each update...
    - Constraints are applied, which eliminate possibilities from each grid cell.
    - Selection is run, which picks a specific tile to use for a single cell.
- If any contradictions occur (a cell ends up with no possible tiles), the generator errors out.
    - There's no backtracking (yet), so the workaround is to re-run the generator, and try to improve constraint and
      tile setups to avoid the likelihood of contradictions.
- All these pieces are basic `UObjects` and can be used manually in various ways if needed.
- Async generation is not yet supported.


- The `UWFCGeneratorComponent` provides a simple interface to run everything from an Actor.
- It requires a `UWFCAsset` as input, which defines all the generator requirements listed above.
- `UWFCAssetModel` (subclass of `UWFCModel`) makes it easier to define tiles using data assets.
    - `UWFCTileSet` contains an array of `UWFCTileAsset` which define things like which actor class to spawn for a tile,
      edge tags for automatically supporting adjacency constraints, etc
    - This model handles expanding these tile assets into the individual tiles using during generation.
        - If a tile asset can be rotated, permutations are created for each rotation.
        - If a tile asset spans more than 1 grid cell (like a big 3x3 piece in a 2D grid), the individual tiles making
          up a big tile are defined, and adjacency rules created to make sure the groups of tiles are selected together.
- The `UWFCGeneratorComponent` only handles running the generator, but a `AWFCTestingActor` is provided as an example
  for spawning tile actors after each grid cell has a tile selected.
    - It's expected that you handle spawning or loading content however you need using the `OnCellSelectedEvent`
      or `OnFinishedEvent` of the generator component.

## Getting Started

- Create a `UWFCAsset`
- Add some constraints, such as an edge and boundary constraint which provide basic rules for respecting adjacency
  rules.
- Pick a cell selector, `WFCEntropyCellSelector` is the most common.
- Select a grid config class (2D or 3D), and set the grid dimensions.
- Create a `UWFCTileSet` data asset and assign it to the WFC asset.
- Create some `UWFCTileAsset2D` or `UWFCTileAsset3D` tiles (whichever matches the grid config) and assign them to the
  set.
- For each tile asset...
    - Create an actor to spawn with some visuals to represent the tile and assign it.
    - Set the edge type tags which will be used by the adjacency constraint to make sure only similar edges are next to
      each other.
- Add a `AWFCTestingActor` to the level and assign the WFC asset to its generator component.
- Hit Play, the WFCTestingActor will automatically run the generator on BeginPlay, and selected tiles will be spawned.

> Check out the `Grid2DTest` or `Grid3DTest` test levels for full examples, as well as their `WFC_Test2D`
> and `WFC_Test3D` example WFC assets.
