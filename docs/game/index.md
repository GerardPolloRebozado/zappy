# Game Design Document

## Definitions

### Player

A player is a physical person connected to the server.

### Inhabitant

An inhabitant is an in-game entity controlled by a set of instructions. This set of instructions can be handled at runtime by our own AIs, or by a player set of codeblocks.

---

# World

## Map

The game world consists of a flat two-dimensional map composed of tiles.

Multiple inhabitants may occupy the same tile simultaneously.

The world contains a finite amount of resources distributed across its tiles.

Future map implementations may introduce terrain types and biome-specific resource distributions. Terrain information is available to inhabitants and AI agents.

## Resources

Resources are distributed evenly across the map.

The total amount of each resource is determined by the following formula:

```text
resource_amount = map_width × map_height × density
```

### Resource Densities

| Resource  | Density |
| --------- | ------- |
| Food      | 0.50    |
| Linemate  | 0.30    |
| Deraumere | 0.15    |
| Sibur     | 0.10    |
| Mendiane  | 0.10    |
| Phiras    | 0.08    |
| Thystame  | 0.05    |

---

# Inhabitants

## Movement and Vision

Movement updates the inhabitant's position and immediately refreshes its field of view.

When an inhabitant completes a move or turn, connected GUI clients are notified so the map view stays in sync with the game state.

The GUI may also query an inhabitant's current position and orientation on demand.

When a GUI client authenticates with `GRAPHIC`, the server replays the current world state as protocol notifications so it starts with a complete picture:

- `pnw` for every connected AI inhabitant (position, orientation, level, team)
- `enw` for every egg still on the map

This complements live `broadcast_event` updates that only reach clients already connected at the time of the event. See `server/src/commands/gui_sync.rs` for the implementation.

Vision range is configurable through the game configuration.

## Inventory

Inventory capacity is unlimited.

An inhabitant cannot inspect the inventory of another inhabitant unless both belong to the same group.

Inventory information may be shared through broadcasting.

## Food

Food represents the lifespan of an inhabitant.

Consuming food immediately increases the inhabitant's remaining lifetime.

Food reserves may provide additional bonuses, such as increased movement speed or increased vision range.

## Broadcasting

Inhabitants may broadcast messages to other inhabitants.

Broadcast messages are used for coordination, communication, and sharing information.

---

# Evolution

Evolution allows inhabitants to increase their level.

To begin an evolution, all required resources must be present on the tile.

All inhabitants on the tile that satisfy the participation requirements take part in the evolution.

Once the evolution ritual begins:

* All participating inhabitants become frozen.
* Frozen inhabitants cannot move or perform actions.
* The ritual continues until it succeeds or is interrupted.

An evolution is interrupted if a required participant is removed from the tile, for example through ejection.

Because evolution can be interrupted, inhabitants benefit from performing evolutions in isolated locations.

## Evolution Requirements

| Target Level | Players Required | Linemate | Deraumere | Sibur | Mendiane | Phiras | Thystame |
| ------------ | ---------------- | -------- | --------- | ----- | -------- | ------ | -------- |
| 4            | 2                | 2        | 0         | 1     | 0        | 2      | 0        |
| 5            | 4                | 1        | 1         | 2     | 0        | 1      | 0        |
| 6            | 4                | 1        | 2         | 1     | 3        | 0      | 0        |
| 7            | 6                | 1        | 2         | 3     | 0        | 1      | 0        |
| 8            | 6                | 2        | 2         | 2     | 2        | 2      | 1        |

---

# Configuration

The game is configured through external configuration files.

Configuration files define game parameters including:

* Map dimensions
* Resource densities
* Vision parameters
* Evolution parameters
* AI strategies
* Gameplay modifiers

Different configuration files allow different game modes and balancing rules without modifying the game code.

