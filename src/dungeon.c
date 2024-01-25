#include <stdio.h>

#include "raylib.h"

#include "dungeon.h"

#define TILETYPE_PTR(name) enum TileType (*name)[N_ROWS]

void print_dungeon(MapData *map) {

    // printf("Sector W: %i\n", SECTOR_W);
    // printf("Sector H: %i\n", SECTOR_H);

	// const enum TileType (*tiles)[N_ROWS] = map->tiles;
    const TILETYPE_PTR(tiles) = map->tiles;

    printf("\n\n");

	for (int row = 0; row < N_ROWS; row++) {
		for (int col = 0; col < N_COLS; col++){
            if (col % SECTOR_W == 0) { printf("| "); continue; }
            if (row % SECTOR_H == 0) { printf("=="); continue; }

			switch (tiles[col][row]) {
				case TILE_TERRAIN: {
					printf("~ ");
					break;
				}
				case TILE_FLOOR: {
					printf("X ");
					break;
				}
				default: {
					break;
				}
			}
		}
		printf("\n");
	}

    printf("\n\n");
}

// subroom
void generate_room(int sX, int sY, int sW, int sH, MapData *map) {
    int lol = 0;

    int minRoomWidth = (int)((float)sW * 0.2f);
    int minRoomHeight = (int)((float)sH * 0.2f);

    printf("Min: [%i, %i]\n", minRoomWidth, minRoomHeight);

    int maxRoomWidth = (int)((float)sW * 0.8f);
    int maxRoomHeight = (int)((float)sH * 0.8f);

    int roomWidth = GetRandomValue(minRoomWidth, maxRoomWidth);
    int roomHeight = GetRandomValue(minRoomHeight, maxRoomHeight);
    printf("Dims: [%i, %i]\n", roomWidth, roomHeight);

    // offsets
    const int gap = 1;
    int oX = GetRandomValue(gap, sW - roomWidth - gap);
    int oY = GetRandomValue(gap, sH - roomHeight - gap);
    printf("Room: (%i,%i) -> (%i, %i)\n", sX + oX, sY + oY, sX + oX + roomWidth, sY + oY + roomHeight);

    TILETYPE_PTR(tiles) = map->tiles;
    for (int col = sX + oX; col < sX + oX + roomWidth; col++) {
        for (int row = sY + oY; row < sY + oY + roomHeight; row++) {
            tiles[col][row] = TILE_FLOOR;
            lol++;
        }
    }

    printf("\nGenerated %i tiles.\n", lol);
}

MapData generate_dungeon() {

    MapData map = { 0 };

    for (int sX = 0; sX < SECTORS_X; sX++) {

        for (int sY = 0; sY < SECTORS_Y; sY++) {

            generate_room(sX * SECTOR_W, sY * SECTOR_H, SECTOR_W, SECTOR_H, &map);

        }

    }


    print_dungeon(&map);
    return map;
}
