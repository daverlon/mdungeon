#pragma once

#include "dungeon.h"

// extern MapGenerationConfig dungeon_preset_basic = (MapGenerationConfig){
//     .n_sectors_x = 2,
//     .n_sectors_y = 2,

//     .sector_cols = 16,
//     .sector_rows = 16,

//     .room_width_min = 4, 
//     .room_width_max = 12, 

//     .room_height_min = 4,
//     .room_height_max = 12,

//     .dummy_chance = 5,
//     .extra_corridor_chance = 3,
//     .corridor_bend_chance = 2
// };

#define DUNGEON_PRESET_BASIC            \
    ((MapGenerationConfig){             \
        .n_sectors_x = 2,               \
        .n_sectors_y = 2,               \
        .sector_cols = 16,              \
        .sector_rows = 16,              \
        .room_width_min = 4,            \
        .room_width_max = 12,           \
        .room_height_min = 4,           \
        .room_height_max = 12,          \
        .dummy_chance = 0,              \
        .extra_corridor_chance = 3,     \
        .corridor_bend_chance = 2       \
    })

// crazy preset lol
// extern MapGenerationConfig dungeon_preset_crazy = (MapGenerationConfig){
//     .n_sectors_x = 3,
//     .n_sectors_y = 3,

//     .sector_cols = 16,
//     .sector_rows = 16,

//     .room_width_min = 4, 
//     .room_width_max = 10, 

//     .room_height_min = 4,
//     .room_height_max = 10,

//     .dummy_chance = 0,
//     .extra_corridor_chance = 1,
//     .corridor_bend_chance = 1
// };