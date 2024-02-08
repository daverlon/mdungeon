#include "dungeon_presets.h"

// first dungeon of the game
void run_intro_dungeon(MapData* mapData) {
    // generate the dungeon layout
    mapData = generate_map(DUNGEON_PRESET_BASIC);
}
