#pragma once

#include "main.h"

#include "dungeon.h"

#include "stdio.h"
#include "stdbool.h"

//
// max tile distance
//
#define MAX_NODES 32

#define DEF_MAX 999

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point parent;
    Point current;
    int g;
    int h;
    int f;
} Node;

typedef struct {
    //Point path[MAX_NODES];
    Point* path;
    int length;
    bool unreachable;
} PathList;

//extern void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList, bool cut_corners);
// extern void dijkstra(MapData* map, Point src, Point dest, PathList* pathList);
// extern void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList);
//extern void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList, bool cut_corners);
//void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList, bool cut_corners, bool ents_block, Entity* ignore);

Point vector2_to_point(Vector2 vec);
Vector2 point_to_vector2(Point point);


extern void astar_around_ents(MapData* map, EntityData* entity_data, PathList* path_list, Point src, Point dest, Entity* src_ent, bool cut_world_corners);
extern void astar_through_ents(MapData* map, PathList* pathList, Point src, Point dest, bool cut_world_corners);



extern bool isInPathList(PathList* pathList, Point p);

PathList find_path_around_ents(Entity* from_ent, Vector2 to_pos, bool cut_world_corners, MapData* map_data, EntityData* entity_data);

PathList find_path_through_ents(Entity* from_ent, Vector2 to_pos, bool cut_world_corners, MapData* map_data);