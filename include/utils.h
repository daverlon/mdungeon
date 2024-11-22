#include "main.h"

Vector2 direction_to_vector2(enum Direction direction);
enum Direction opposite_direction(enum Direction direction);
enum Direction vector_to_direction(Vector2 vector);
Vector2 get_tile_infront_entity(const Entity* ent);

int value_variation(float value, int percentage);
Vector2 get_active_position(Entity* ent);


void print_vector2(Vector2 vec);
Vector2 position_to_grid_position(Vector2 pos);