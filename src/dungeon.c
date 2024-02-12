#include <stdio.h>

//#include "raylib.h"

//#include "dungeon.h"

#include "main.h"
#include "dungeon.h"

void print_map(int cols, int rows, enum TileType tiles[MAX_COLS][MAX_ROWS]) {
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            printf("%d ", tiles[col][row]);
        }
        printf("\n");
    }
}

void connect_rooms(Room *room1, Room *room2, enum TileType tiles[MAX_COLS][MAX_ROWS], int bend_chance) {
    // Calculate the centers of the two rooms
    int center1_x = room1->x + room1->cols / 2;
    int center1_y = room1->y + room1->rows / 2;

    int center2_x = room2->x + room2->cols / 2;
    int center2_y = room2->y + room2->rows / 2;

    // Create a corridor with bends
    int col = center1_x;
    int row = center1_y;

    while (col != center2_x || row != center2_y) {
        if (tiles[col][row] == TILE_WALL) {
            // Mark the wall or corridor as corridor
            tiles[col][row] = TILE_CORRIDOR;
        }
        else if (tiles[col][row] == TILE_FLOOR) {
            // Mark the tiles where a corridor meets a room as entrance points
            if ((col > room1->x && col < room1->x + room1->cols - 1 && row == room1->y) ||
                (col > room1->x && col < room1->x + room1->cols - 1 && row == room1->y + room1->rows - 1) ||
                (row > room1->y && row < room1->y + room1->rows - 1 && col == room1->x) ||
                (row > room1->y && row < room1->y + room1->rows - 1 && col == room1->x + room1->cols - 1) ||
                (col > room2->x && col < room2->x + room2->cols - 1 && row == room2->y) ||
                (col > room2->x && col < room2->x + room2->cols - 1 && row == room2->y + room2->rows - 1) ||
                (row > room2->y && row < room2->y + room2->rows - 1 && col == room2->x) ||
                (row > room2->y && row < room2->y + room2->rows - 1 && col == room2->x + room2->cols - 1)) {
                tiles[col][row] = TILE_ROOM_ENTRANCE;
            }
            else {
                tiles[col][row] = TILE_FLOOR;
            }
        }

        // Introduce random bends
        if (bend_chance != 0) {
            if (GetRandomValue(0, bend_chance) == 0) {
                // Move vertically
                if (row < center2_y) {
                    row++;
                } else if (row > center2_y) {
                    row--;
                }
            } 
            else {
                // Move horizontally
                if (col < center2_x) {
                    col++;
                } else if (col > center2_x) {
                    col--;
                }
            }
        }
        else {
            // tiles[col][row] = TILE_CORRIDOR;

            // Move toward the center of the second room
            if (col < center2_x) {
                col++;
            }
            else if (col > center2_x) {
                col--;
            }
            else if (row < center2_y) {
                row++;
            }
            else if (row > center2_y) {
                row--;
            }
        }
    }
    room1->n_corridors++;
    room2->n_corridors++;
}

// /*void*/enum TileType** generate_map(MapGenerationConfig config) {
MapData generate_map(MapGenerationConfig config) {

    MapData ret = { 0 };

    const int n_sectors_x = config.n_sectors_x;
    const int n_sectors_y = config.n_sectors_y;

    // n_sectors_x * n_sectors_y
    const int n_sectors = n_sectors_x * n_sectors_y; 

    printf("N Sectors: %i, %i: (%i)\n", n_sectors_x, n_sectors_y, n_sectors);

    const int sector_rows = config.sector_rows;
    const int sector_cols = config.sector_cols;

    ret.cols = sector_cols * n_sectors_x;
    if (ret.cols > MAX_COLS) {
        printf("Error: Map cols (%i) exceeds MAX_COLS (%i)\n", ret.cols, MAX_COLS);
    }
    ret.rows = sector_rows * n_sectors_y;
    if (ret.rows > MAX_ROWS) {
        printf("Error: Map rows (%i) exceeds MAX_ROWS (%i)\n", ret.cols, MAX_ROWS);
    }

    // enum TileType tiles[n_sectors_x * sector_cols][n_sectors_y * sector_rows];
    // enum TileType (*tiles)[MAX_ROWS] = ret.tiles;
    // init
    for (int col = 0; col < MAX_COLS; col++) {
        for (int row = 0; row < MAX_ROWS; row++) {
            ret.tiles[col][row] = TILE_WALL;
        }
    }
    // printf("LOL\n");

    Room rooms[MAX_ROOMS];
    for (int i = 0; i < n_sectors; i++) {
        rooms[i] = (Room){
            .x = 0,
            .y = 0,
            .cols = 0,
            .rows = 0,
            .n_corridors = 0
        };
    }
    
    // generate rooms
    for (int sector_x = 0; sector_x < n_sectors_x; sector_x++) {
        for (int sector_y = 0; sector_y < n_sectors_y; sector_y++) {

            int sector_index = sector_x * n_sectors_y + sector_y;
            printf("Room index: %i\n", sector_index);
            bool dummy = GetRandomValue(0, config.dummy_chance) == 0;
            if ((config.dummy_chance == 0) ||
                (config.dummy_chance != 0 && !dummy)) {
                //
                // create room in this sector
                //
                int room_width = GetRandomValue(config.room_width_min, config.room_width_max);
                int room_height = GetRandomValue(config.room_height_min, config.room_height_max);

                int room_x = GetRandomValue(sector_x * sector_cols, (sector_x + 1) * sector_cols - room_width);
                int room_y = GetRandomValue(sector_y * sector_rows, (sector_y + 1) * sector_rows - room_height);

                // int room_x = GetRandomValue(sector_x * sector_cols, sector_x * sector_cols + sector_cols - room_width);
                // int room_y = GetRandomValue(sector_y * sector_rows, sector_y * sector_rows + sector_rows - room_height);
                printf("Spawn room %i in sector: %i, %i, Room: %i, %i, %i %i\n", sector_index, sector_x, sector_y, room_x, room_y, room_width, room_height);
                rooms[sector_index] = (Room){
                    .x = room_x,
                    .y = room_y,
                    .cols = room_width,
                    .rows = room_height,
                    .n_corridors = 0,
                };
            }
            else if ((config.dummy_chance != 0 && dummy)) {
                if (1 == 1) {
                    // create dummy room in this sector
                    // int dummy_x = GetRandomValue(sector_x * sector_cols, sector_cols);
                    // int dummy_y = GetRandomValue(sector_y * sector_rows, sector_rows);
                    int dummy_x = GetRandomValue(sector_x * sector_cols, sector_x * sector_cols + sector_cols);
                    int dummy_y = GetRandomValue(sector_y * sector_rows, sector_y * sector_rows + sector_rows);
                    printf("Spawn dummy room at %i, %i\n", dummy_x, dummy_y);
                    rooms[sector_index] = (Room){
                        .x = dummy_x, 
                        .y = dummy_y,
                        .cols = 1, 
                        .rows = 1,
                        .n_corridors = 0,
                    };
                }
            }
        }
    }

    printf("N SECTORS: %i\n", n_sectors);
    for (int i = 0; i < n_sectors; i++) {
        Room* rm = &rooms[i];
        printf("Room %i: [%i, %i, %i, %i]\n", i, rm->x, rm->y, rm->cols, rm->rows);
        for (int col = rm->x; col < (rm->x + rm->cols); col++) {
            for (int row = rm->y; row < (rm->y + rm->rows); row++) {
                // printf("floor %i,%i\n", col, row);
                ret.tiles[col][row] = TILE_FLOOR;

                //if (col == rm->x || col == (rm->x + rm->cols - 1) || row == rm->y || row == (rm->y + rm->rows - 1)) {
                //    // Mark room entrances as TILE_ROOM_ENTRANCE
                //    ret.tiles[col][row] = TILE_ROOM_ENTRANCE;
                //}
                //else {
                //    ret.tiles[col][row] = TILE_FLOOR;
                //}

            }
        }
    }

    for (int i = 0; i < n_sectors; i++) {
        for (int j = i + 1; j < n_sectors; j++) {
            Room *room1 = &rooms[i];
            Room *room2 = &rooms[j];
            
            printf("Room %i -> %i\n", i, j);
            
            if (!room1->n_corridors || !room2->n_corridors) {
                // if (room1->n_corridors > 0 && room2->n_corridors > 0) {
                // printf("* Connecting room %i -> %i\n", i, j);
                connect_rooms(room1, room2, ret.tiles, config.corridor_bend_chance);
            }
            else {
                //
                // at least 1 corridor on each room
                //

                //
                // if it is a dead-end, force another corridor
                // (avoid extra long dead-ends)
                //
                if ((room1->cols == 1 && room1->rows == 1)
                ||  (room2->cols == 1 && room2->rows == 1)) {
                    connect_rooms(room1, room2, ret.tiles, config.corridor_bend_chance);
                }

                // printf("*** Connecting room %i -> %i\n", i, j);
                if ((config.extra_corridor_chance != 0 && 
                    GetRandomValue(0, config.extra_corridor_chance) == 0)
                    // ||
                    // config.extra_corridor_chance == 0) {
                ) {
                        connect_rooms(room1, room2, ret.tiles, config.corridor_bend_chance);
                }
            }
        }
    }

    //
    // force dead-end to loop
    for (int i = 0; i < n_sectors; i++) {
        for (int j = i + 1; j < n_sectors; j++) {
            Room *room1 = &rooms[i];
            Room *room2 = &rooms[j];
            
            if (
                ((room1->cols == 1 && room1->rows == 1) && room1->n_corridors < 2)
                ||
                ((room2->cols == 1 && room2->rows == 1) && room2->n_corridors < 2)
            ) {
                printf("[Dead-end detection] Forcing connection between room %i (%i,%i) and room %i (%i, %i)\n",
                    i, room1->x, room1->y, j, room2->x, room2->y);
                connect_rooms(room1, room2, ret.tiles, config.corridor_bend_chance);
            }
        }
    }


    // printf("Map size: %ix%i\n", ret.cols, ret.rows);
    for (int row = 0; row < ret.rows; row++) {
        for (int col = 0; col < ret.cols; col++){
        // printf("Col: %i\n", col);
            // if (row % sector_rows == 0) { printf("| "); continue; }
            // if (col % sector_cols == 0) { printf("=="); continue; }

			switch (ret.tiles[col][row]) {
				case TILE_WALL: {
					printf("~ ");
					break;
				}
				case TILE_FLOOR: {
					printf("X ");
					break;
				}
                case TILE_CORRIDOR: {
                    printf("O ");
                    break;
                }
                case TILE_ROOM_ENTRANCE: {
                    printf("E ");
                    break;
                }
                // case TILE_CORRIDOR_MEETING_POINT: {
                //     printf("M ");
                //     break;
                // }
				default: {
                    printf("Error: Unknown tile type found on map. (%i, %i) = %i\n", col, row, ret.tiles[col][row]);
                    printf("? ");
					break;
				}
			}
		}
		printf("\n");
	}     
    for (int i = 0; i < n_sectors; i++) {
        printf("Room %i: %i cors\n", i, rooms[i].n_corridors);
    }

    // ret.tiles = tiles;
    return ret;
    // return tiles;
    
}
