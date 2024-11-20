#include "utils.h"

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