#pragma once

enum TileType {
    TILE_INVALID, // error
    TILE_WALL, // default terrain
    TILE_FLOOR,
    TILE_CORRIDOR,
    TILE_ROOM_ENTRANCE
    // TILE_CORRIDOR_MEETING_POINT
};

typedef struct {
    enum TileType type;
    bool found;
    //bool reserved;
} TileData;

// typedef struct {
//     enum TileType type;

// } TileData;

typedef struct {
    // can check if dummy by checking if
    //              cols == 1 && rows == 1
    //              this should be fine
    // bool is_dummy;
    int x;
    int y;
    int cols;
    int rows;
    // bool has_corridor;
    int n_corridors;
    // bool skip; // no room here
} Room;

#define MAX_ROOMS 32

/*
    int n_sectors_x;
    int n_sectors_y;

    int sector_rows;
    int sector_cols;

    int room_width_min;
    int room_height_min;
    int room_width_max;
    int room_height_max;

    //
    // probabilities
    // all GetRandomValue(0,n)==0
    //
    int dummy_chance; 
    int extra_corridor_chance; 
    int corridor_bend_chance;
*/
typedef struct {
    int n_sectors_x;
    int n_sectors_y;

    int sector_cols;
    int sector_rows;

    int room_width_min;
    int room_width_max;

    int room_height_min;
    int room_height_max;

    //
    // probabilities
    // all GetRandomValue(0,n)==0
    //
    // todo: think of a better way to do this?
    // however it works well as is
    // 
    int dummy_chance; 
    int extra_corridor_chance; 
    int corridor_bend_chance;

} MapGenerationConfig;

#define MAX_ROWS 128
#define MAX_COLS 128

typedef struct {
    int n_sectors;

    int cols;
    int rows;
    // enum TileType** tileenum TileType (*tiles)[rows];s;
    // enum TileType (*tiles)[MAX_ROWS];

    //enum TileType tiles[MAX_COLS][MAX_ROWS];
    TileData tiles[MAX_COLS][MAX_ROWS];

    //Vector2 corridor_entrance_points[32];
    // enum TileType (*tiles)[MAX_ROWS];
    Room rooms[MAX_ROOMS];

    float view_distance;
} MapData;

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

