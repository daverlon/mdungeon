#pragma once

enum TileType {
    TILE_TERRAIN,
    TILE_FLOOR,
    TILE_CORRIDOR,
    TILE_CORRIDOR_MEETING_POINT
};


// #define N_ROWS 48
// #define N_COLS 48

// #define SECTORS_X 2
// #define SECTORS_Y 2

// #define SECTOR_W (N_COLS/SECTORS_X)
// #define SECTOR_H (N_ROWS/SECTORS_Y)

typedef struct {
    int map_cols;
    int map_rows;
    int n_sectors_x;
    int n_sectors_y;
    enum TileType *tiles;
} MapData;

typedef struct {
    // bool is_dummy;
    int x;
    int y;
    int cols;
    int rows;
    bool has_corridor;
} Room;

extern void generate_small_groves_map();
// extern MapData generate_dungeon();
// extern MapData generate_dungeon();
