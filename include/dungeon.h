#pragma once

#include "main.h"

extern MapData generate_map(MapGenerationConfig config);

extern int get_room_id_at_position(int x, int y, MapData* map_data);


// #define TILETYPE_PTR(name) enum TileType (**name)


#define DUNGEON_PRESET_BASIC            \
    ((MapGenerationConfig){             \
        .n_sectors_x = 2,               \
        .n_sectors_y = 2,               \
        .sector_cols = 16,              \
        .sector_rows = 16,              \
        .room_width_min = 4,            \
        .room_width_max = 10,           \
        .room_height_min = 4,           \
        .room_height_max = 10,          \
        .dummy_chance = 0,              \
        .extra_corridor_chance = 3,     \
        .corridor_bend_chance = 2       \
    })

#define DUNGEON_PRESET_ADVANCED         \
    ((MapGenerationConfig){             \
        .n_sectors_x = 3,               \
        .n_sectors_y = 3,               \
        .sector_cols = 16,              \
        .sector_rows = 16,              \
        .room_width_min = 6,            \
        .room_width_max = 12,           \
        .room_height_min = 6,           \
        .room_height_max = 12,          \
        .dummy_chance = 3,              \
        .extra_corridor_chance = 3,     \
        .corridor_bend_chance = 2       \
    })

