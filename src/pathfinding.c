#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
// #include <math.h>

#include "raymath.h"

#include "pathfinding.h"

void initNodeList(NodeList* list) {
    list->size = 0;
    // for (size_t i = 0; i < MAX_NODES; i++) {
    //     list->data[i] = (Node){0, 0}; //with your actual default value
    // }
}

void pushNode(NodeList* list, Node node) {
    if (list->size < MAX_NODES) {
        list->data[list->size++] = node;
    }
}

Node popNode(NodeList* list) {
    return list->data[--list->size];
}

bool isEmpty(NodeList* list) {
    return list->size == 0;
}

int calculateHeuristic(Node a, Node b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

bool isValid(int x, int y, int cols, int rows) {
    return x >= 0 && x < cols && y >= 0 && y < rows;
}

bool inList(NodeList* list, Node node) {
    size_t s = list->size;
    for (size_t i = 0; i < s; i++) {
        if (list->data[i].x == node.x && list->data[i].y == node.y) {
            return true;
        }
    }
    return false;
}

void print_node(Node node) {
	printf("Node {%i, %i}\n", node.x, node.y);
}

NodeList findPath(int cols, int rows, Node start, Node end, enum TileType tiles[MAX_COLS][MAX_ROWS]) {
// NodeList findPath(int cols, int rows, Node start, Node end, enum TileType tiles[MAX_COLS][MAX_ROWS]) {

    if (start.x == end.x && start.y == end.y) {
        // Start and end positions are the same, return an empty path.
        NodeList empty_path = {0, NULL};
        return empty_path;
    }

    NodeList openSet, closedSet;
    initNodeList(&openSet);
    initNodeList(&closedSet);

    pushNode(&openSet, start);

    int gScore[MAX_COLS][MAX_ROWS];
    int fScore[MAX_COLS][MAX_ROWS];

    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            gScore[i][j] = 9999;
            fScore[i][j] = 9999;
        }
    }

    gScore[start.x][start.y] = 0;
    fScore[start.x][start.y] = calculateHeuristic(start, end);


    while (!isEmpty(&openSet)) {
        Node current = popNode(&openSet);

        if (current.x == end.x && current.y == end.y) {
            // Reconstruct path
            NodeList path = { 0, NULL };
            initNodeList(&path);
            Node temp = current;
            pushNode(&path, temp);
            while (temp.x != start.x || temp.y != start.y) {
                temp = closedSet.data[temp.y * cols + temp.x];  // Corrected indexing
                pushNode(&path, temp);
            }

            return path;
        }

        // Iterate over neighbors
        int dx[] = {-1, 0, 1, 0};
        int dy[] = {0, 1, 0, -1};
        for (int i = 0; i < 4; i++) {
            int neighborX = current.x + dx[i];
            int neighborY = current.y + dy[i];

            if (!isValid(neighborX, neighborY, cols, rows))
                continue;

            // Skip blocked or non-traversable tiles
            // if (tiles[neighborX][neighborY] != TILE_FLOOR)
            if (tiles[neighborX][neighborY] == TILE_WALL)
                continue;

            int tentativeGScore = gScore[current.x][current.y] + 1;

            if (tentativeGScore < gScore[neighborX][neighborY]) {
                gScore[neighborX][neighborY] = tentativeGScore;
                fScore[neighborX][neighborY] = tentativeGScore + calculateHeuristic(
                    (Node){neighborX, neighborY},
                    end
                );

                Node neighbor = {neighborX, neighborY};

                // Update parent node in closedSet
                closedSet.data[neighborY * rows + neighborX] = current;

                if (!inList(&openSet, neighbor)) {
                    pushNode(&openSet, neighbor);
                }
            }

            // printf("hehe\n");
        }
    }

    NodeList emptyList;
    initNodeList(&emptyList);
    return emptyList; // No path found
}