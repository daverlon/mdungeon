#include "pathfinding.h"

#include "utils.h"
#include "main.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
// #include <math.h>


#define INT_MAX 9999

extern bool any_entity_exists_on_tile(int col, int row, const EntityData* entity_data, Entity* ignore, int* out);

bool isValid(Point point, int cols, int rows) {
    return (point.x >= 0 && point.x < cols&& point.y >= 0 && point.y < rows);
}

bool isUnblocked(MapData* map, Point point) {
    return (map->tiles[point.x][point.y].type != TILE_WALL);
}

bool isDestination(Point current, Point dest) {
    return (current.x == dest.x && current.y == dest.y);
}

int calculateHValue(Point current, Point dest) {
    return abs(current.x - dest.x) + abs(current.y - dest.y);
}

void tracePath(Node** nodeDetails, Point dest, PathList* pathList) {
    int row = nodeDetails[dest.y][dest.x].parent.y;
    int col = nodeDetails[dest.y][dest.x].parent.x;

    while (!(nodeDetails[row][col].parent.x == col && nodeDetails[row][col].parent.y == row)) {
        pathList->length++;
        pathList->path = realloc(pathList->path, pathList->length * sizeof(Point));
        pathList->path[pathList->length - 1] = (Point){ col, row };

        int tempRow = nodeDetails[row][col].parent.y;
        col = nodeDetails[row][col].parent.x;
        row = tempRow;
    }
}

void freePathList(PathList* pathList) {
    free(pathList->path);
    pathList->path = NULL;
    pathList->length = 0;
}

bool isInPathList(PathList* pathList, Point p) {
    for (int i = 0; i < pathList->length; i++) {
        if (p.x == pathList->path[i].x &&
            p.y == pathList->path[i].y) return true;
    }
    return false;
}

Point vector2_to_point(Vector2 vec) {
    return (Point) { (int)vec.x, (int)vec.y };
}

Vector2 point_to_vector2(Point point) {
    return (Vector2) { point.x, point.y };
}

void astar_around_ents(MapData* map, EntityData* entity_data, PathList* path_list, Point src, Point dest, Entity* src_ent, bool cut_world_corners) {

    //printf("%i, %i\n", src.x, src.y);
    if (!isValid(src, map->cols, map->rows) || !isValid(dest, map->cols, map->rows)) {
        printf("Invalid source or destination\n");
        return;
    }

    if (!isUnblocked(map, src) || !isUnblocked(map, dest)) {
        printf("Source or destination is blocked\n");
        return;
    }

    if (isDestination(src, dest)) {
        //printf("Source is the destination\n");
        return;
    }

	bool closedList[MAX_COLS][MAX_ROWS] = { 0 };
    Node** nodeDetails = (Node**)malloc(map->rows * sizeof(Node*));
    for (int i = 0; i < map->rows; ++i) {
        nodeDetails[i] = (Node*)malloc(map->cols * sizeof(Node));
    }

    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            closedList[col][row] = false;
            nodeDetails[row][col].f = INT_MAX;
            nodeDetails[row][col].g = INT_MAX;
            nodeDetails[row][col].h = INT_MAX;
            nodeDetails[row][col].parent.x = -1;
            nodeDetails[row][col].parent.y = -1;
        }
    }

    int startX = src.x;
    int startY = src.y;
    nodeDetails[startY][startX].f = 0;
    nodeDetails[startY][startX].g = 0;
    nodeDetails[startY][startX].h = 0;
    nodeDetails[startY][startX].parent.x = startX;
    nodeDetails[startY][startX].parent.y = startY;

    while (true) {
        int minF = INT_MAX;
        Point current = { 0 };

        for (int row = 0; row < map->rows; ++row) {
            for (int col = 0; col < map->cols; ++col) {
                if (!closedList[col][row] && nodeDetails[row][col].f < minF) {
                    minF = nodeDetails[row][col].f;
                    current.x = col;
                    
                    current.y = row;
                }
            }
        }

        if (minF == INT_MAX) {
            //printf("Destination not reachable\n");
            path_list->unreachable = true;
            break;
        }

        closedList[current.x][current.y] = true;

        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                Point neighbor = { current.x + i, current.y + j };

                if (isValid(neighbor, map->cols, map->rows) && isUnblocked(map, neighbor)) {
                    // Skip the center tile
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    // Check for corner cutting prevention
                    if (!cut_world_corners && (i != 0 && j != 0)) {
                        // Check if adjacent tiles are unblocked
                        if (!isUnblocked(map, (Point) { current.x + i, current.y }) ||
                            !isUnblocked(map, (Point) { current.x, current.y + j })) {
                            continue;  // Skip diagonal movement if cutting corners
                        }
                    }

                    // check if ent is present on this tile
                    bool b = false;
                    for (int i = 0; i < entity_data->entity_counter; i++) {
                        Entity* e = &entity_data->entities[i];
                        if (e == src_ent) continue;
                        Vector2 invalid_pos = { 0 };
                        // check if entity is moving
                        if (e->state == MOVE) {
                            invalid_pos = get_tile_infront_entity(e);
                        }
                        
                        if (e->state != MOVE) {
                            invalid_pos = e->original_position;
                        }

                        if (Vector2Equals(invalid_pos, point_to_vector2(current))) {
                            b = true;
                        }
                    }
                    if (b)
                        continue;

                    int gNew = nodeDetails[current.y][current.x].g + 1;

                    // Adjust the cost for diagonal movements
                    if (i != 0 && j != 0) {
                        gNew = nodeDetails[current.y][current.x].g + 1.4;  // Diagonal cost
                    }

                    int hNew = calculateHValue(neighbor, dest);
                    int fNew = gNew + hNew;

                    if (isValid(neighbor, map->cols, map->rows) &&
                        !closedList[neighbor.x][neighbor.y] &&
                        isUnblocked(map, neighbor) &&
                        fNew < nodeDetails[neighbor.y][neighbor.x].f) {
                        nodeDetails[neighbor.y][neighbor.x].f = fNew;
                        nodeDetails[neighbor.y][neighbor.x].g = gNew;
                        nodeDetails[neighbor.y][neighbor.x].h = hNew;
                        nodeDetails[neighbor.y][neighbor.x].parent.x = current.x;
                        nodeDetails[neighbor.y][neighbor.x].parent.y = current.y;
                    }
                }
            }
        }

        if (isDestination(current, dest)) {
            //printf("Destination reached!\n");
            tracePath(nodeDetails, dest, path_list);
            break;
        }
    }

    // Free allocated memory
    for (int i = 0; i < map->rows; ++i) {
        free(nodeDetails[i]);
    }
    free(nodeDetails);
}

void astar_through_ents(MapData* map, PathList* pathList, Point src, Point dest, bool cut_world_corners) {
    //printf("%i, %i\n", src.x, src.y);
    if (!isValid(src, map->cols, map->rows) || !isValid(dest, map->cols, map->rows)) {
        printf("Invalid source or destination\n");
        return;
    }

    if (!isUnblocked(map, src) || !isUnblocked(map, dest)) {
        //printf("Source or destination is blocked\n");
        return;
    }

    if (isDestination(src, dest)) {
        //printf("Source is the destination\n");
        return;
    }

    bool closedList[MAX_COLS][MAX_ROWS] = { 0 };
    Node** nodeDetails = (Node**)malloc(map->rows * sizeof(Node*));
    for (int i = 0; i < map->rows; ++i) {
        nodeDetails[i] = (Node*)malloc(map->cols * sizeof(Node));
    }

    for (int row = 0; row < map->rows; ++row) {
        for (int col = 0; col < map->cols; ++col) {
            closedList[col][row] = false;
            nodeDetails[row][col].f = INT_MAX;
            nodeDetails[row][col].g = INT_MAX;
            nodeDetails[row][col].h = INT_MAX;
            nodeDetails[row][col].parent.x = -1;
            nodeDetails[row][col].parent.y = -1;
        }
    }

    int startX = src.x;
    int startY = src.y;
    nodeDetails[startY][startX].f = 0;
    nodeDetails[startY][startX].g = 0;
    nodeDetails[startY][startX].h = 0;
    nodeDetails[startY][startX].parent.x = startX;
    nodeDetails[startY][startX].parent.y = startY;

    while (true) {
        int minF = INT_MAX;
        Point current = { 0 };

        for (int row = 0; row < map->rows; ++row) {
            for (int col = 0; col < map->cols; ++col) {
                if (!closedList[col][row] && nodeDetails[row][col].f < minF) {
                    minF = nodeDetails[row][col].f;
                    current.x = col;
                    current.y = row;
                }
            }
        }

        if (minF == INT_MAX) {
            //printf("Destination not reachable\n");
            pathList->unreachable = true;
            break;
        }

        closedList[current.x][current.y] = true;

        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                Point neighbor = { current.x + i, current.y + j };

                if (isValid(neighbor, map->cols, map->rows) && isUnblocked(map, neighbor)) {
                    // Skip the center tile
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    // Check for corner cutting prevention
                    if (!cut_world_corners && (i != 0 && j != 0)) {
                        // Check if adjacent tiles are unblocked
                        if (!isUnblocked(map, (Point) { current.x + i, current.y }) ||
                            !isUnblocked(map, (Point) { current.x, current.y + j })) {
                            continue;  // Skip diagonal movement if cutting corners
                        }
                    }

                    int gNew = nodeDetails[current.y][current.x].g + 1;

                    // Adjust the cost for diagonal movements
                    if (i != 0 && j != 0) {
                        gNew = nodeDetails[current.y][current.x].g + 1.4;  // Diagonal cost
                    }

                    int hNew = calculateHValue(neighbor, dest);
                    int fNew = gNew + hNew;

                    if (isValid(neighbor, map->cols, map->rows) &&
                        !closedList[neighbor.x][neighbor.y] &&
                        isUnblocked(map, neighbor) &&
                        fNew < nodeDetails[neighbor.y][neighbor.x].f) {
                        nodeDetails[neighbor.y][neighbor.x].f = fNew;
                        nodeDetails[neighbor.y][neighbor.x].g = gNew;
                        nodeDetails[neighbor.y][neighbor.x].h = hNew;
                        nodeDetails[neighbor.y][neighbor.x].parent.x = current.x;
                        nodeDetails[neighbor.y][neighbor.x].parent.y = current.y;
                    }
                }
            }
        }

        if (isDestination(current, dest)) {
            //printf("Destination reached!\n");
            tracePath(nodeDetails, dest, pathList);
            break;
        }
    }

    // Free allocated memory
    for (int i = 0; i < map->rows; ++i) {
        free(nodeDetails[i]);
    }
    free(nodeDetails);
}

PathList find_path_around_ents(Entity* from_ent, Vector2 to_pos, bool cut_world_corners, MapData* map_data, EntityData* entity_data) {
        PathList path_list = { .path = NULL, .length = 0 };

        astar_around_ents(
            map_data,
            entity_data,
            &path_list,
            vector2_to_point(from_ent->original_position),
            vector2_to_point(to_pos),
            from_ent,
            cut_world_corners);

        return path_list;
}

PathList find_path_through_ents(Entity* from_ent, Vector2 to_pos, bool cut_world_corners, MapData* map_data) {
    PathList path_list = { .path = NULL, .length = 0 };

    astar_through_ents(
        map_data,
        &path_list,
        vector2_to_point(from_ent->original_position),
        vector2_to_point(to_pos),
        cut_world_corners);

    return path_list;
}
