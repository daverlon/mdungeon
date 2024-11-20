#include "utils.h"

#include <stdio.h>

Vector2 direction_to_vector2(enum Direction direction) {
    switch (direction) {
    case DOWN:
        return (Vector2) { 0.0f, 1.0f };
    case DOWNRIGHT:
        return (Vector2) { 1.0f, 1.0f };
    case RIGHT:
        return (Vector2) { 1.0f, 0.0f };
    case UPRIGHT:
        return (Vector2) { 1.0f, -1.0f };
    case UP:
        return (Vector2) { 0.0f, -1.0f };
    case UPLEFT:
        return (Vector2) { -1.0f, -1.0f };
    case LEFT:
        return (Vector2) { -1.0f, 0.0f };
    case DOWNLEFT:
        return (Vector2) { -1.0f, 1.0f };
    default:
        // Handle invalid direction
        return (Vector2) { 0.0f, 0.0f };
    }
}

enum Direction opposite_direction(enum Direction direction) {
    switch (direction) {
    case DOWN:
        return UP;
    case DOWNRIGHT:
        return UPLEFT;
    case RIGHT:
        return LEFT;
    case UPRIGHT:
        return DOWNLEFT;
    case UP:
        return DOWN;
    case UPLEFT:
        return DOWNRIGHT;
    case LEFT:
        return RIGHT;
    case DOWNLEFT:
        return UPRIGHT;
    default:
        // Handle invalid direction by returning a default value or an error
        return direction; // or return a default direction like UP
    }
}

enum Direction vector_to_direction(Vector2 vector) {
    if (vector.x == 0.0f && vector.y == 1.0f) {
        return DOWN;
    }
    else if (vector.x == 1.0f && vector.y == 1.0f) {
        return DOWNRIGHT;
    }
    else if (vector.x == 1.0f && vector.y == 0.0f) {
        return RIGHT;
    }
    else if (vector.x == 1.0f && vector.y == -1.0f) {
        return UPRIGHT;
    }
    else if (vector.x == 0.0f && vector.y == -1.0f) {
        return UP;
    }
    else if (vector.x == -1.0f && vector.y == -1.0f) {
        return UPLEFT;
    }
    else if (vector.x == -1.0f && vector.y == 0.0f) {
        return LEFT;
    }
    else if (vector.x == -1.0f && vector.y == 1.0f) {
        return DOWNLEFT;
    }
    return DOWN;
}

Vector2 get_tile_infront_entity(const Entity* ent) {
    return Vector2Add(ent->original_position, direction_to_vector2(ent->direction));
}

int value_variation(float value, int percentage) {
    int v = GetRandomValue(-percentage, percentage);
    float vf = 1.0 + (v / 100.0);

    float fval = roundf(value);
    fval *= vf;
    return (int)roundf(fval);
}

Vector2 get_active_position(Entity* ent) {
    switch (ent->state) {
    case MOVE:
        //return ent->position;
        return get_tile_infront_entity(ent);
        //return ent->position;
        break;
        /*case ATTACK_MELEE:
            return ent->original_position;*/
        break;
    default:
        break;
    }
    if (ent->is_swapping) return get_tile_infront_entity(ent);
    return ent->original_position;
}

void print_vector2(Vector2 vec) {
    printf("Vector2: (%.8f, %.8f)\n", vec.x, vec.y);
}

Vector2 position_to_grid_position(Vector2 pos) {
    Vector2 gridPos = Vector2Multiply(pos, (Vector2) { TILE_SIZE, TILE_SIZE });

    // Center the entity within the grid cell
    gridPos.x += (TILE_SIZE - SPRITE_SIZE) / 2;
    gridPos.y += (TILE_SIZE - SPRITE_SIZE) / 2;
    gridPos.y -= SPRITE_SIZE / 4;

    return gridPos;
}