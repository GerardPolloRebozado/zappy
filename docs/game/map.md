# The Cartographer's Ledger: The Endless Horizon of Trantor

Hark, ye bold navigators and brass-shod explorers! You stand upon the threshold of Trantor, a world of celestial clockwork and ether-seas.

## The Great Toroidal Loop
Trantor is a curious jewel floating in the Great Ether-Stream. To sail off the Eastern edge is but to find oneself emerging from the West; to march beyond the Northern frost is to feel the Southern sun upon your back. 'Tis an endless horizon where no man is ever truly lost.
The map is a 2D grid of size `W` by `H`. Coordinates wrap using modulo: `New_Coord = (Current + Move) % Max`. The shortest distance is calculated as `Δx = min(|x1 - x2|, W - |x1 - x2|)` and `Δy = min(|y1 - y2|, H - |y1 - y2|)`.

---

## Biomes: Realm of the Four Winds

Trantor is divided into distinct provinces, each governed by the ancient alchemy of the planet’s core.

### 1. The Obsidian Barrens
Sharp as a duelist’s blade and dark as a pirate’s heart, these volcanic plains are the bones of the world. The earth bleeds gems, but the soil is barren of Sun-Bread, and a hungry belly makes for a short journey.
`linemate_density = base * 1.5`, `deraumere_density = base * 1.5`, `food_density = base * 0.2`.

### 2. The Luminous Orchards
A forest of bioluminescent spores where the very air tastes of honey and light. However, the thick "Light-Fog" clings to the soul, narrowing your sight by a league.
`food_density = base * 2.0`, `vision_range = max(1, base_vision - 1)`.

### 3. The Crystal Canyons
Massive, singing geodes that pierce the heavens, acting as natural resonators for the Ether. Your "Broadcast-Cries" carry 50% further through the singing crystalline air.
`thystame_density = base * 1.8`, `phiras_density = base * 1.5`, `broadcast_range_multiplier = 1.5`.

### 4. The Magnetic Tundra
A freezing wasteland where the lodestones of the world go mad, dancing to the tune of unseen magnets. Electromagnetic storms play tricks on the ear, whisking voices away so they seem to come from elsewhere.
`broadcast_direction_variance = rand(-1, 1)` shifts the reported direction `K` by -1, 0, or +1.

---

## Celestial Anomalies: Graces of the Void

The heavens above Trantor are as restless as the sea. At any moment, the stars may grant a boon or a curse.

### Meteor Shower
A rain of stellar fire that leaves the tiles choked with random gems.
Spawns `(W * H * 0.05)` random stones every 2 turns for a duration of 50 turns.

### Solar Flare
A blinding burst of clarity that grants all Squires "All-Seeing Sight," yet drains their Sun-Bread at double speed.
`global_vision = MAX_INT`, `food_consumption_rate = 2x`. Duration: 20 turns.

### Gravity Well
A vortex of pull and pressure, dragging all nearby souls one league closer to its center every few turns.
Every 5 turns, moves all inhabitants within radius `R = min(W, H) / 4` one tile closer to the "Well" tile `(Gx, Gy)`. Duration: 30 turns.

### Psionic Echo
A ghostly whisper from the ancestors that hastens the Great Ascension for those already in the midst of the ritual.
All currently active **Incantations** immediately have their remaining time reduced by 25%.

---

## Legendary Landmarks

### Ancient Monoliths
Brass-bound towers of old, built by those who sailed the Ether before us. To perform a ritual upon such a tile is to be blessed with the "Grip of the Titan" or the "Wind’s Speed."
Successful Incantations on Monolith tiles grant a `Speed_Buff` (next 10 actions are 20% faster) or an `Inventory_Expansion`.

### Ether-Fissures (Wormholes)
Rifts in the fabric of space itself. Step into a Fissure, and find yourself spat out across the world in an eyeblink.
Linked pairs `(F1, F2)`. A `Forward` command into `F1` teleports the inhabitant to `F2`'s position instantly.

---

## The Planet's Exhalation
Trantor does not merely "spawn" riches; she breathes. Every 20 ticks, the world "exhales" a pulse of minerals. But take heed: if too many Squires crowd a single isle, the planet grows shy, and the harvest shall be thin.
Every 20 time units, if `current_count < target_density`, the server spawns the difference. However, tiles with more than 4 inhabitants have a `spawn_chance` of 0 for that pulse cycle.
