#pragma once

enum TileType {
    TILE_INVALID, // error
    TILE_WALL, // default terrain
    TILE_FLOOR,
    TILE_CORRIDOR,
    TILE_ROOM_ENTRANCE
    // TILE_CORRIDOR_MEETING_POINT
};

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

#define MAX_ROWS 64
#define MAX_COLS 64

typedef struct {
    int cols;
    int rows;
    // enum TileType** tileenum TileType (*tiles)[rows];s;
    // enum TileType (*tiles)[MAX_ROWS];
    enum TileType tiles[MAX_COLS][MAX_ROWS];
    Vector2 corridor_entrance_points[32];
    // enum TileType (*tiles)[MAX_ROWS];
} MapData;

extern MapData generate_map(MapGenerationConfig config);


// #define TILETYPE_PTR(name) enum TileType (**name)
