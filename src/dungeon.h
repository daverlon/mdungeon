#pragma once

enum TileType {
    TILE_TERRAIN,
    TILE_FLOOR
};

#define N_ROWS 48
#define N_COLS 48

#define SECTORS_X 2
#define SECTORS_Y 2

#define SECTOR_W (N_COLS/SECTORS_X)
#define SECTOR_H (N_ROWS/SECTORS_Y)

typedef struct {
    enum TileType tiles[N_COLS][N_ROWS];
} MapData;

extern MapData generate_dungeon();