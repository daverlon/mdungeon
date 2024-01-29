#pragma once

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
} Node;

typedef struct {
    Node data[MAX_NODES];
    size_t size;
} NodeList;

extern void print_node(Node node);
extern bool inList(NodeList* list, Node node);
extern NodeList findPath(int cols, int rows, Node start, Node end, enum TileType tiles[MAX_COLS][MAX_ROWS]);