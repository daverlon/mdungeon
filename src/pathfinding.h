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
} PathList;

//extern void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList, bool cut_corners);
// extern void dijkstra(MapData* map, Point src, Point dest, PathList* pathList);
// extern void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList);
//extern void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList, bool cut_corners);
//void aStarSearch(MapData* map, Point src, Point dest, PathList* pathList, bool cut_corners, bool ents_block, Entity* ignore);


extern void astar_around_ents(MapData* map, EntityData* entity_data, PathList* path_list, Point src, Point dest, Entity* src_ent, bool cut_world_corners);
extern void astar_through_ents(MapData* map, PathList* pathList, Point src, Point dest, bool cut_world_corners);



extern bool isInPathList(PathList* pathList, Point p);
