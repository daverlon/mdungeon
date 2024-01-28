#include <stdio.h>

#include "raylib.h"

#include "dungeon.h"

#define TILETYPE_PTR(name) enum TileType (*name)[N_ROWS]

void print_dungeon(MapData *map) {

    // printf("Sector W: %i\n", SECTOR_W);
    // printf("Sector H: %i\n", SECTOR_H);

	// // const enum TileType (*tiles)[N_ROWS] = map->tiles;
    // const TILETYPE_PTR(tiles) = map->tiles;

    // printf("\n\n");

	// for (int row = 0; row < N_ROWS; row++) {
	// 	for (int col = 0; col < N_COLS; col++){
    //         if (col % SECTOR_W == 0) { printf("| "); continue; }
    //         if (row % SECTOR_H == 0) { printf("=="); continue; }

	// 		switch (tiles[col][row]) {
	// 			case TILE_TERRAIN: {
	// 				printf("~ ");
	// 				break;
	// 			}
	// 			case TILE_FLOOR: {
	// 				printf("X ");
	// 				break;
	// 			}
    //             case TILE_ROOM_ENTRANCE: {
    //                 printf("O ");
    //                 break;
    //             }
	// 			default: {
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	printf("\n");
	// }

    // printf("\n\n");
}

// todo: think of a better name
// first dungeon: Small Groves
// idea: difficulty increase -> Deep Small Groves
//       with more complex layouts
void generate_small_groves_map() {
    // const int n_sectors_x = GetRandomValue(2, 3);
    // const int n_sectors_y = GetRandomValue(2, 3);
    const int n_sectors_x = 2;
    const int n_sectors_y = 2;


    const int n_sectors = n_sectors_x * n_sectors_y;

    printf("N Sectors: %i, %i: (%i)\n", n_sectors_x, n_sectors_y, n_sectors);

    const int sector_rows = 16;
    const int sector_cols = 16;

    enum TileType tiles[n_sectors_x * sector_cols][n_sectors_y * sector_rows];
    // init
    for (int col = 0; col < n_sectors_x * sector_cols; col++){
            for (int row = 0; row < n_sectors_y * sector_rows; row++) {
                tiles[col][row] = TILE_TERRAIN;
            }
    }

    Room rooms[n_sectors]; // todo: initialize
    
    // generate rooms
    for (int sector_x = 0; sector_x < n_sectors_x; sector_x++) {
        for (int sector_y = 0; sector_y < n_sectors_y; sector_y++) {

            int sector_index = sector_x * n_sectors_y + sector_y;
            bool dummy = GetRandomValue(0,100) == 0;
            if (!dummy) {
                //
                // create room in this sector
                //
                int room_width = GetRandomValue(4, 8);
                int room_height = GetRandomValue(4, 8);
                int room_x = GetRandomValue(sector_x * sector_cols, sector_x * sector_cols + sector_cols - room_width);
                int room_y = GetRandomValue(sector_y * sector_rows, sector_y * sector_rows + sector_rows - room_height);
                printf("Spawn room in sector: %i, %i, Room: %i, %i, %i %i\n", sector_x, sector_y, room_x, room_y, room_width, room_height);
                rooms[sector_index] = (Room){
                    room_x,
                    room_y,
                    room_width,
                    room_height,
                    false
                };
                //
            }
            else {
                // create dummy room in this sector
                // int dummy_x = GetRandomValue(sector_x * sector_cols, sector_cols);
                // int dummy_y = GetRandomValue(sector_y * sector_rows, sector_rows);
                int dummy_x = GetRandomValue(sector_x * sector_cols, sector_x * sector_cols + sector_cols);
                int dummy_y = GetRandomValue(sector_y * sector_rows, sector_y * sector_rows + sector_rows);
                rooms[sector_index] = (Room){
                    dummy_x, 
                    dummy_y,
                    1, 
                    1,
                    false
                };
            }
        }
    }

    printf("N SECTORS: %i\n", n_sectors);
    for (int i = 0; i < n_sectors; i++) {
        Room* rm = &rooms[i];
        printf("Room %i: [%i, %i, %i, %i]\n", i, rm->x, rm->y, rm->cols, rm->rows);
        for (int col = rm->x; col < (rm->x + rm->cols); col++) {
            for (int row = rm->y; row < (rm->y + rm->rows); row++) {
                printf("floor %i,%i\n", col, row);
                tiles[col][row] = TILE_FLOOR;
            }
        }
    }

    for (int i = 0; i < n_sectors_x * n_sectors_y; i++) {
        Room* cur_room = &rooms[i];
        
        // left, right, top, bottom 
        int adj[4] = { -1, -1, -1, -1 };

        // Check if there is a sector to the left
        if (i % n_sectors_x != 0) {
            adj[0] = 1;  // Left side
            printf("Sector %i has left-side\n", i);
            int leftRoomIndex = (i % n_sectors_x != 0) ? i - 1 : -1;  // Left side
            // create corridor
            Room* left_room = &rooms[leftRoomIndex];
            
            int left_delta = left_room->x - cur_room->x;
            int meeting_point_col = GetRandomValue(left_room->x + left_room->cols + 1, cur_room->x - 1);
            int highest = cur_room->y < left_room->y ? cur_room->y : left_room->y;
            int lowest = (cur_room->y + cur_room->rows) > (left_room->y + left_room->rows) ? 
                cur_room->y + cur_room->rows : left_room->y + left_room->rows;
            int meeting_point_row = GetRandomValue(lowest, highest);
            tiles[meeting_point_col][meeting_point_row] = TILE_CORRIDOR_MEETING_POINT;
            cur_room->has_corridor = true;

            int left_room_entrance_y = GetRandomValue(left_room->y, left_room->y + left_room->rows);
            int cur_room_entrance_y = GetRandomValue(cur_room->y, cur_room->y + cur_room->rows);
            //
            // create corridor
            //
            // start from left room

            // create corridor from left room -> meeting point
            for (int corridor_col = left_room->x + left_room->cols; corridor_col < meeting_point_col; corridor_col++) {
                tiles[corridor_col][left_room_entrance_y] = TILE_CORRIDOR;
            }
            // create corridor from cur room (right) -> meeting point
            for (int corridor_col = cur_room->x; corridor_col > meeting_point_col; corridor_col--) {
                tiles[corridor_col][left_room_entrance_y] = TILE_CORRIDOR;
            }
            int y_delta = left_room_entrance_y - meeting_point_row;
            int step = (y_delta < 0) ? -1 : 1;
            // go up/down to meeting_point_row
            for (int row = meeting_point_row; row != left_room_entrance_y; row += step) {
                tiles[meeting_point_col-1][row] = TILE_CORRIDOR;
                tiles[meeting_point_col+1][row] = TILE_CORRIDOR;
            }
        }

        // Check if there is a sector to the right
        if ((i + 1) % n_sectors_x != 0) {
            adj[1] = 1;  // Right side
            printf("Sector %i has right-side\n", i);
            int rightRoomIndex = ((i + 1) % n_sectors_x != 0) ? i + 1 : -1;  // Right side
        }

        // Check if there is a sector above
        if (i >= n_sectors_x) {
            adj[2] = 1;  // Top side
            printf("Sector %i has top-side\n", i);
            int topRoomIndex = (i >= n_sectors_x) ? i - n_sectors_x : -1;  // Top side
        }

        // Check if there is a sector below
        if (i < n_sectors - n_sectors_x) {
            adj[3] = 1;  // Bottom side
            printf("Sector %i has bottom-side\n", i);
            int bottomRoomIndex = (i < n_sectors - n_sectors_x) ? i + n_sectors_x : -1;  // Bottom side
        }
    }
    
        for (int row = 0; row < n_sectors_y * sector_rows; row++) {
    for (int col = 0; col < n_sectors_x * sector_cols; col++){
        // printf("Col: %i\n", col);
            // if (row % sector_rows == 0) { printf("| "); continue; }
            // if (col % sector_cols == 0) { printf("=="); continue; }

			switch (tiles[col][row]) {
				case TILE_TERRAIN: {
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
                case TILE_CORRIDOR_MEETING_POINT: {
                    printf("M ");
                    break;
                }
				default: {
                    printf("? ");
					break;
				}
			}
		}
		printf("\n");
	}     
}

// // subroom
// void generate_room(int sX, int sY, int sW, int sH, MapData *map) {

//     int minRoomWidth = (int)((float)sW * 0.2f);
//     // int minRoomHeight = (int)((float)sH * 0.2f);
//     int minRoomHeight = (int)((float)minRoomWidth * 0.3f);

//     printf("Min: [%i, %i]\n", minRoomWidth, minRoomHeight);

//     int maxRoomWidth = (int)((float)sW * 0.8f);
//     // int maxRoomHeight = (int)((float)sH * 0.8f);
//     // int maxRoomHeight = (int)((float)maxRoomWidthf);
//     int maxRoomHeight = maxRoomWidth;

//     int roomWidth = GetRandomValue(minRoomWidth, maxRoomWidth);
//     int roomHeight = GetRandomValue(minRoomHeight, maxRoomHeight);
//     printf("Dims: [%i, %i]\n", roomWidth, roomHeight);

//     // offsets
//     const int gap = 1;
//     int oX = GetRandomValue(gap, sW - roomWidth - gap);
//     int oY = GetRandomValue(gap, sH - roomHeight - gap);
//     printf("Room: (%i,%i) -> (%i, %i)\n", sX + oX, sY + oY, sX + oX + roomWidth, sY + oY + roomHeight);

//     TILETYPE_PTR(tiles) = map->tiles;
//     for (int col = sX + oX; col < sX + oX + roomWidth; col++) {
//         for (int row = sY + oY; row < sY + oY + roomHeight; row++) {
//             tiles[col][row] = TILE_FLOOR;
//         }
//     }

    // find out what sector this is
    // int sXn = (int)((float)sX / (float)sW); // sector x value
    // int sYn = (int)((float)sY / (float)sY); // sector y value

    // generate entrances

    // create entrance
    // if the room is the top left
    // todo: 
    // if (sXi == 0 && sYi == 0) {
    //     // how many entrances?
    //     int n_entrances = GetRandomValue(1, 2);
    //     if (n_entrances == 1) {
    //         int right_or_bottom = GetRandomValue(0, 1);
    //         if (right_or_bottom == 0) {
    //             // right
    //             int y_entrance_pos = GetRandomValue(sY + oY, sY + oY + roomHeight);
    //             tiles[sX + oX + roomWidth-1][y_entrance_pos] = TILE_ROOM_ENTRANCE;
    //         } 
    //         else if (right_or_bottom == 1) {
    //             // bottom
    //             int x_entrance_pos = GetRandomValue(sX + oX, sX + oX + roomWidth);
    //             tiles[x_entrance_pos][sY + oY + roomHeight-1] = TILE_ROOM_ENTRANCE;
    //         }
    //     } 
    //     else if (n_entrances == 2) {
            
    //     }
    // }
// }

// MapData generate_dungeon() {

//     MapData map = { 0 };

//     for (int sX = 0; sX < SECTORS_X; sX++) {

//         for (int sY = 0; sY < SECTORS_Y; sY++) {

//             generate_room(sX * SECTOR_W, sY * SECTOR_H, SECTOR_W, SECTOR_H, &map);

//         }

//     }


//     print_dungeon(&map);
//     return map;
// }
