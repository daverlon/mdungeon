/*
    Philosophy:
    Code the game. Nothing more.

    "just type the code for those entities." - Jon Blow
    https://youtu.be/4oky64qN5WI?t=415
*/

#include <stdio.h>
// #include <string.h>

#include "main.h"

#include "dungeon.h"
#include "dungeon_presets.h"

#include "pathfinding.h"

void print_vector2(Vector2 vec) {
    printf("Vector2: (%.8f, %.8f)\n", vec.x, vec.y);
}

#define BLACK_SEMI_TRANSPARENT (Color){0, 0, 0, 128}
#define WHITE_SEMI_TRANSPARENT (Color){255, 255, 255, 64}
#define GREEN_SEMI_TRANSPARENT (Color){0, 255, 50, 32}
#define LIGHTBLUE (Color){50, 100, 200, 150}
#define LIGHTGREEN (Color){50, 200, 100, 250}
#define LIGHTYELLOW (Color){100, 100, 50, 100}

const int world_width = 32 * 14;

#define LOAD_ZOR_TEXTURE() (LoadTexture("res/zor/zor_spritesheet.png"))
#define LOAD_FANTANO_TEXTURE() (LoadTexture("res/fantano/fantano_idle.png"))
#define LOAD_CYHAR_TEXTURE() (LoadTexture("res/cyhar/cyhar_idle.png"))

// mobs
#define LOAD_FLY_TEXTURE() (LoadTexture("res/entities/fly.png"))

#define LOAD_SPILLEDCUP_TEXTURE() (LoadTexture("res/items/item_spilledcup.png"))
#define LOAD_STICK_TEXTURE() (LoadTexture("res/items/item_stick.png"))
#define LOAD_APPLE_TEXTURE() (LoadTexture("res/items/item_apple.png"))

#define LOAD_FOREST_GRASS_TILES_TEXTURE() (LoadTexture("res/environment/floor_forest_grass.png"))
#define LOAD_FOREST_GRASS_DARK_TILES_TEXTURE() (LoadTexture("res/environment/floor_grass_blue.png"))
#define LOAD_FOREST_TERRAIN_TEXTURE() (LoadTexture("res/environment/terrain_forest_bush.png"))
#define LOAD_FOREST_DIRT_TILES_TEXTURE() (LoadTexture("res/environment/floor_dirt.png"))
#define LOAD_FOREST_ACTIVE_GRASS() (LoadTexture("res/environment/floor_active_grass.png"))

#define LOAD_DESERT_TILES_TEXTURE() (LoadTexture("res/environment/floor_sand.png"))

#define GET_LAST_ENTITY_REF() (&entity_data.entities[entity_data.entity_counter-1])

// #define NPC_MAX_INVENTORY_SIZE 4

// todo: rotation animation
// timed rotations
enum Direction {
    DOWN,
    DOWNRIGHT,
    RIGHT,
    UPRIGHT,
    UP,
    UPLEFT,
    LEFT,
    DOWNLEFT
};

void rotate_smooth(enum Direction target, enum Direction *dir) {
    int diff = (target - *dir + 8) % 8; // Calculate the shortest difference in direction

    if (diff == 0)
        return; // If the target is the same as the current direction, no rotation needed

    int steps = (diff <= 4) ? diff : 8 - diff; // Choose the smaller rotation direction

    for (int i = 0; i < steps; i++) {
        *dir = (*dir + 1) % 8; // Rotate one step clockwise
        // You can add code here to visualize or animate the rotation
    }
}

void print_turn_queue(int turn_queue[], int queue_size) {
    printf("Turn Queue: [");
    for (int i = 0; i < queue_size; ++i) {
        printf("%d", turn_queue[i]);
        if (i < queue_size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

// turn queue (queue size should be limited to max entities per turn)
typedef struct {
    Entity entity;

} Turn;

typedef struct {
    enum ItemType type;
    Vector2 position; // grid coordinate position
    // for player to ignore item once dropped
    // enemies should still be able to pickup the item (perhaps some won't want to though)
    //bool prevent_pickup;  // default 0: (0=can pickup) (moved to entity)
} Item;
// ideas:   BasicItem (items with consistent effects)
//          SpecialItem (items that may change? this seems weird..)
//          ItemWear (wear value for each item (such that they are disposable? probably not fun)) 

typedef struct {
    Item items[MAX_INSTANCES];
    int item_counter;
} ItemData;

//enum DungeonTextureLayer {
//    LAYER_FLOOR,
//    //LAYER_TERRAIN, // terrain layer will be rendered onto floor layer anyway?
//    LAYER_FLOOR_ABOVE_MODELS
//};
//
//typedef struct {
//    RenderTexture textures[2]; // layers above
//} DungeonTexture;
//#define DUNGEON_TEXTURE_LAYERS 2

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
}

void set_entity_position(Entity* ent, Vector2 pos) {
    ent->position = pos;
    ent->original_position = pos;
}


Vector2 get_tile_infront_entity(Entity* ent) {
    return Vector2Add(ent->original_position, direction_to_vector2(ent->direction));
}

bool entity_reached_destination(Entity* ent) {
    return Vector2Equals(ent->position, ent->original_position);
}

bool item_exists_on_tile(int col, int row, const ItemData* item_data) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < item_data->item_counter; i++) {
        if (Vector2Equals(item_data->items[i].position, pos)) {
            return true;
        }
    }
    return false;
}

bool any_entity_exists_on_tile(int col, int row, const EntityData* entity_data, Entity* ignore, int* out) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < entity_data->entity_counter; i++) {
        Entity* ent = &entity_data->entities[i];

        if (ignore != NULL && ent == ignore) continue;

        Vector2 ent_target_position = get_tile_infront_entity(ent);
        if (
			     (ent->state == MOVE && Vector2Equals(ent_target_position, pos))
              || (Vector2Equals(ent->original_position, pos))
              ) {
            if (out != NULL) {
                *out = i;
            }
            return true;
        }

    }
    return false;
}

bool entity_exists_on_tile(int col, int row, Entity* ent) {
    Vector2 pos = (Vector2){ col, row };
	Vector2 ent_target_position = get_tile_infront_entity(ent);
	if (
		/* (ent->state == IDLE && Vector2Equals(ent->original_position, pos)
	  || */(ent->state == MOVE && Vector2Equals(ent_target_position, pos)
		  || (Vector2Equals(ent->original_position, pos))
		  )) {
		return true;
	}
    return false;
}

void reset_entity_state(Entity* ent, bool use_turn) {
    ent->state = IDLE;
    ent->original_position = ent->position;
    if (use_turn)
		ent->n_turn++;
}

void move_entity_forward(Entity* ent) {
    //if (ent->state != MOVE) return;

    const Vector2 movement = direction_to_vector2(ent->direction);
    //const Vector2 normalized_movement = Vector2Normalize(movement); // Normalize the movement vector
    const Vector2 targ = Vector2Add(ent->original_position, movement);

    float distance_to_target = Vector2Distance(ent->position, targ);

    float max_move_distance = GetFrameTime() * GRID_MOVESPEED;

    if (max_move_distance >= distance_to_target) {
        ent->position = targ;
        reset_entity_state(ent, true);
    }
    else {
        ent->position.x += movement.x * max_move_distance;
        ent->position.y += movement.y * max_move_distance;
    }
}

void swap_entity_positions(Entity* ent1, Entity* ent2) {

    Vector2 ent1_direction = Vector2Subtract(ent2->position, ent1->position);
    Vector2 ent2_direction = Vector2Subtract(ent1->position, ent2->position);

    if (ent1->state == IDLE && ent2->state == IDLE) {
        ent1->direction = vector_to_direction(ent1_direction);
        ent2->direction = vector_to_direction(ent2_direction);

        // Swap target positions
        /*ent1->target_position = ent2->position;
        ent2->target_position = ent1->position;*/
    }

    // Set entities to move
    ent1->state = MOVE;
    ent2->state = MOVE;
}

void control_entity(Entity* ent, const enum TileType tiles[MAX_COLS][MAX_ROWS], EntityData* entity_data) {

    // if no key is held, ensure that isMoving is set to false
    if (ent->state != IDLE) return;

    bool should_move = false;

    if (IsKeyDown(KEY_S)) {
        if (IsKeyDown(KEY_A)) {
            ent->direction = DOWNLEFT;
        }
        else if (IsKeyDown(KEY_D)) {
            ent->direction = DOWNRIGHT;
        }
        else {
            ent->direction = DOWN;
        }
        should_move = true;
    }

    if (IsKeyDown(KEY_W)) {
        if (IsKeyDown(KEY_A)) {
            ent->direction = UPLEFT;
        }
        else if (IsKeyDown(KEY_D)) {
            ent->direction = UPRIGHT;
        }
        else {
            ent->direction = UP;
        }
        should_move = true;
    }

    if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) {
        ent->direction = LEFT;
        should_move = true;
    }
    else if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) {
        ent->direction = RIGHT;
        should_move = true;
    }

    if (IsKeyDown(KEY_SPACE) && should_move) {
        should_move = false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !should_move) {
        ent->state = ATTACK_MELEE;
        ent->animation.cur_frame = 0;
    }

    // this may be a bit scuffed. unecessary extra checks here?
    Vector2 movement = direction_to_vector2(ent->direction);
    
    if (should_move) {
        if (ent->prevent_pickup)
            ent->prevent_pickup = false;

        // printf("MOVE!\n");
        // printf("X: %2.0f -> %2.0f\n", ent->position.x, ent->position.x+1.0f);
        // printf("Y: %2.0f -> %2.0f\n", ent->position.y, ent->position.y+1.0f);
        Vector2 target_position = Vector2Add(ent->original_position, movement);
        // printf("Target: %2.5f, %2.5f\n", ent->targetPosition.x, ent->targetPosition.y);
        int i_targ_x = (int)target_position.x;
        int i_targ_y = (int)target_position.y;
        // printf("Target: %i, %i\n", i_targ_x, i_targ_y);

        //
        // todo: fix diagonal tiles
        // idea: walking on flame tiles or other
        //       hazard tiles apply debufs/affects to player
        //       tradeoff for walking on unsafe tile
        //
        // diagonal block
        if (movement.x != 0.0f && movement.y != 0.0f) {
            if (tiles[i_targ_x][(int)ent->position.y] == TILE_WALL
            ||  tiles[(int)ent->position.x][i_targ_y] == TILE_WALL) {
                reset_entity_state(ent, false);
                return;
            }
        }

        // entity block/swap
        int swapi = -1;
        if (entity_exists_on_tile(i_targ_x, i_targ_y, entity_data, ent, &swapi, false)) {
            //printf("Tried to walk on tile with entity\n");
            if (!entity_data->entities[swapi].can_swap_positions) {
                /*ent->should_move = false;
                ent->state = IDLE;*/
                //ent->state = IDLE;
                //should_move = false;
                reset_entity_state(ent, false);
                return;
            }
            //else if (entity_data->entities[swapi].can_swap_positions) {
            //    swap_entity_positions(en, &entity_data->entities[swapi]);
            //   /* ent->should_move = false;
            //    ent->state = IDLE;*/
            //    reset_entity_state(en, true);
            //    reset_entity_state(&entity_data->entities[swapi], true);
            //    return;
            //}
           /* else if (&entity_data->entities[swapi]) {
                swap_entity_positions(en, &entity_data->entities[swapi]);
                return;
            }*/
        }

        switch (tiles[i_targ_x][i_targ_y]) {
            case TILE_WALL: {
                // printf("Invalid movement.!\n");
                reset_entity_state(ent, false);
                return;
                break;
            }
            case TILE_ROOM_ENTRANCE:
            case TILE_CORRIDOR:
            case TILE_FLOOR: {
                //reset_entty_state(en, false);
                ent->state = MOVE;
                break;
            }
            default: {
                printf("Error (MOVEMENT) invalid tile detected?: %i\n", tiles[i_targ_x][i_targ_y]);
                break;
            }
        }
    }
}

bool move_entity_in_direction(Entity* ent, enum Direction dir) {
    const Vector2 movement = direction_to_vector2(dir);

    float max_move_distance = GetFrameTime() * GRID_MOVESPEED;

    float distance_to_target = Vector2Distance(ent->position, movement);

    if (max_move_distance > distance_to_target) {
        max_move_distance = distance_to_target;
    }

    if (max_move_distance >= distance_to_target) {
        reset_entity_state(ent, true);
        return true;
    }
    else {
        ent->position.x += movement.x * max_move_distance;
        ent->position.y += movement.y * max_move_distance;
    }
    return false;
}

void move_entity_freely(Entity* ent) {
    // move entity (no grid involved)
    if (ent->state != MOVE) return;

    Vector2 movement = direction_to_vector2(ent->direction);

    ent->position.x += movement.x * GetFrameTime() * GRID_MOVESPEED;
    ent->position.y += movement.y * GetFrameTime() * GRID_MOVESPEED;

    ent->state = IDLE;
}

void update_animation(Animation* anim) {
    anim->cur_frame_time += GetFrameTime();

    // calculate the number of frames to advance based on elapsed time
	int frames_to_advance = (int)(anim->cur_frame_time / anim->max_frame_time);
	anim->cur_frame_time -= frames_to_advance * anim->max_frame_time;

	anim->cur_frame = (anim->cur_frame + frames_to_advance) % anim->n_frames;

	if (anim->cur_frame % anim->n_frames == 0) {
		anim->cur_frame = 0;
	}
}

Vector2 position_to_grid_position(Vector2 pos) {
    Vector2 gridPos = Vector2Multiply(pos, (Vector2) { TILE_SIZE, TILE_SIZE });

    // Center the entity within the grid cell
    gridPos.x += (TILE_SIZE - SPRITE_SIZE) / 2;
    gridPos.y += (TILE_SIZE - SPRITE_SIZE) / 2;
    gridPos.y -= SPRITE_SIZE / 4;

    return gridPos;
}

void render_entity(Entity* ent) {
    Vector2 grid_position = position_to_grid_position(ent->position);
    Vector2 grid_original_position = position_to_grid_position(ent->original_position);
    Vector2 grid_infront_position = position_to_grid_position(get_tile_infront_entity(ent));
    // printf("[%i,%i] -> [%i,%i]\n", (int)ent->position.x, (int)ent->position.y, (int)gridPosition.x, (int)gridPosition.y);
    // printf("%i\n", ent->animation.yOffset);

    // offset y
    DrawCircle(
        grid_position.x + (SPRITE_SIZE / 2.0f),
        grid_position.y + (SPRITE_SIZE / 2.0f) + (SPRITE_SIZE / 4.0f),
        35.0f,
        BLACK_SEMI_TRANSPARENT
    );
    DrawRectangle(
        grid_original_position.x + (SPRITE_SIZE / 2.0f),
        grid_original_position.y + (SPRITE_SIZE / 2.0f) + (SPRITE_SIZE / 4.0f),
        15,
        15,
        RED
    );
    DrawRectangle(
        grid_infront_position.x + (SPRITE_SIZE / 2.0f),
        grid_infront_position.y + (SPRITE_SIZE / 2.0f) + (SPRITE_SIZE / 4.0f),
        15,
        15,
        BLUE
    );

    // DrawRectangleLines(
    //  gridPosition.x, gridPosition.y, SPRITE_SIZE, SPRITE_SIZE, BLACK);
    DrawTextureRec(
        ent->texture,
        (Rectangle) {
        ent->animation.cur_frame* SPRITE_SIZE,
            ent->animation.y_offset + (ent->direction * SPRITE_SIZE),
            SPRITE_SIZE,
            SPRITE_SIZE
    },
        grid_position,
            WHITE
            );
}

void lunge_entity(Entity* ent) {

    const Vector2 movement = direction_to_vector2(ent->direction);

    float lunge_distance = 1.0f;
    float lunge_speed = 2.5f;
    float lunge_distance_this_frame = lunge_speed * GetFrameTime();

    if (ent->lunge_progress < lunge_distance / 2) {
        ent->position = Vector2Add(ent->position, Vector2Scale(movement, lunge_distance_this_frame));
    }
    else {
        ent->position = Vector2Subtract(ent->position, Vector2Scale(movement, lunge_distance_this_frame));
    }

    ent->lunge_progress += lunge_distance_this_frame;

    if (ent->lunge_progress >= lunge_distance) {
        ent->lunge_progress = 0.0f;
        ent->state = IDLE;
        //ent->position = (Vector2){ roundf(ent->position.x), roundf(ent->position.y) };
        //ent->original_position = (Vector2){ roundf(ent->position.x), roundf(ent->position.y) };
        ent->position = ent->original_position;
        reset_entity_state(ent, true);
    }
}

void update_animation_state(Entity* ent) {
    switch (ent->ent_type) {
    case ENT_ZOR: {
        switch (ent->state) {
			case IDLE: {
				ent->animation.max_frame_time = 0.02f;
				ent->animation.y_offset = 0;
				break;
			}
			case MOVE: {
				ent->animation.max_frame_time = 0.030f;
				ent->animation.y_offset = 2048;
				break;
			}
			case ATTACK_MELEE: {
				ent->animation.max_frame_time = 0.017f;
				ent->animation.y_offset = 2048 + 2048;
				break;
			}
			default: {
				break;
			}
        }
        break;
	}
    case ENT_FANTANO: {
        switch (ent->state) {
			case IDLE: {
				ent->animation.max_frame_time = 0.017f;
				ent->animation.y_offset = 0;
				break;
			}
			case ATTACK_MELEE: {
				break;
			}
			default: {
				break;
			}
        }
        break;
    }
    case ENT_CYHAR: {
        switch (ent->state) {
        case IDLE: {
            ent->animation.max_frame_time = 0.019f;
            ent->animation.y_offset = 0;
            break;
        }
        case ATTACK_MELEE: {
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    case ENT_FLY: {
        switch (ent->state) {
			case IDLE: {
				ent->animation.max_frame_time = 0.037f;
				ent->animation.y_offset = 0;
				break;
			}
			case ATTACK_MELEE: {
				ent->animation.max_frame_time = 0.010f;
				ent->animation.y_offset = 2048;
				break;
			}
			default: {
				break;
			}
		}
        break;
    }
    default:
        break;
    }
	update_animation(&ent->animation);
}

Vector2 find_random_empty_floor_tile(const MapData* map_data, const ItemData* item_data, const EntityData* entity_data) {
    int col = -1;
    int row = -1;

    while (1) {
        // choose tile
		col = GetRandomValue(0, MAX_COLS);
		row = GetRandomValue(0, MAX_ROWS);

        // check if tile is TILE_FLOOR
        if (map_data->tiles[col][row] != TILE_FLOOR) 
            continue;

        // check if item exists on tile
        if (item_exists_on_tile(col, row, item_data))
            continue;

        if (entity_exists_on_tile(col, row, entity_data, NULL, NULL, true))
            continue;

        // todo:
        // check if entity exists on tile

        break;
	}
	return (Vector2) { col, row };
}

void create_item_instance(Item item, ItemData* item_data) {
    //printf("Lol: %i\n", *counter);
    if (item_data->item_counter >= MAX_INSTANCES) {
        printf("Cannot spawn item. Maximum items reached.\n");
        return;
    }
    if (item.type == ITEM_NOTHING) {
        printf("Error: Tried to create ITEM_NOTHING.\n");
        return;
    }
    item_data->items[item_data->item_counter] = item;
    item_data->item_counter++;
    printf("Created item. Counter: %i\n", item_data->item_counter);
}

void delete_item(const int index, ItemData* item_data) {
    if (index >= item_data->item_counter) {
        printf("Warning: Tried to delete item index: %i, instance counter: %i\n", index, item_data->item_counter);
        return;
    }

    for (int i = index; i < item_data->item_counter; i++) {
        item_data->items[i] = item_data->items[i + 1];
    }
    item_data->item_counter--;
    printf("Deleted item index: %i, %i instances left on map.\n", index, item_data->item_counter);
}

void spawn_items(enum ItemType item_type, ItemData* item_data, const MapData* map_data, int min, int max) {
    if (item_data->item_counter >= MAX_INSTANCES || item_data->item_counter + max >= MAX_INSTANCES) {
        printf("Error: [Spawn Items] mapItemCounter already at maximum instances.\n");
        return;
    }

    int n_items = GetRandomValue(min, max);
    int item_counter = 0;
    while (item_counter < n_items) {
        //for (int i = 0; i < n_spilledcups; i++) {
        int col = GetRandomValue(0, MAX_COLS);
        int row = GetRandomValue(0, MAX_ROWS);
        if (map_data->tiles[col][row] != TILE_FLOOR)
            continue;
        // tile is floor
        bool position_taken = false;
        for (int i = 0; i < n_items; i++) {
            if (Vector2Equals(
                item_data->items[i].position,
                (Vector2) {
                col, row
            })) {
                position_taken = true;
                break;
            }
        }
        if (position_taken) continue;
        printf("Creating spilledcup at %i, %i\n", col, row);
        // found valid tile
        create_item_instance((Item) { item_type, (Vector2) { col, row } }, item_data);
        item_counter++;
    }
}

void nullify_all_items(ItemData* item_data) {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        item_data->items[i] = (Item){ ITEM_NOTHING, (Vector2) { 0, 0 }};
    }
    item_data->item_counter = 0;
    printf("Set every item to ITEM_NOTHING\n");
}

// all items should probably not be on 1 texture.
// only certain items can appear at a time
// todo: un-generic this function
// void render_items_on_map(int* counter, Item* items, Texture2D tx) {
    // requires textures 
// }

void pickup_item(const int index, ItemData* item_data, Entity* entity) {
    // add item to entity inventory
    // this should be called with delete_item called after
    // todo:
    // check for out of bounds index (should never occur with good code)
    if (entity->inventory_item_count < INVENTORY_SIZE) {
        entity->inventory[entity->inventory_item_count] = item_data->items[index].type;
        entity->inventory_item_count++;
    }
    delete_item(index, item_data);
    // else: "inventory full"?
}

void scan_items_for_pickup(ItemData* item_data, Entity* entity) {
    if (!entity->prevent_pickup) {
		// entity can pickup item
        for (int i = 0; i < item_data->item_counter; i++) {
            Item* item = &item_data->items[i];
            // item exists on same tile as entity
            if (Vector2Equals(item->position, entity->position)) {
                pickup_item(i, item_data, entity);
            }
        }
	}
}

void delete_item_from_entity_inventory(const int index, Entity* entity) {
    if (index >= entity->inventory_item_count) {
        printf("Warning: Tried to delete inventory item index: %i, item counter: %i\n", index, entity->inventory_item_count);
        return;
    }

    for (int i = index; i < entity->inventory_item_count; i++) {
        entity->inventory[i] = entity->inventory[i + 1];
    }
    entity->inventory_item_count--;
    printf("Deleted inventory item index: %i, %i items remaining.\n", index, entity->inventory_item_count);
}

void drop_item(const int index, Entity* entity, ItemData* item_data) {
    //if (entity->state != IDLE) {
    if (entity->state == MOVE) {
        printf("Tried to drop item while entity is moving.\n");
        return;
    }
    // check if item already exists at current position 
    for (int i = 0; i < item_data->item_counter; i++) {
        if (Vector2Equals(entity->position, item_data->items[i].position)) {
            printf("Tried to drop item on top of existing item.\n");
            return;
        }
    }
    item_data->items[item_data->item_counter] = (Item){ entity->inventory[index], entity->position };
    item_data->item_counter++;
    delete_item_from_entity_inventory(index, entity);
    entity->prevent_pickup = true;
}

void render_player_inventory(Entity* player) {
    // for debug/development purposes
    const int c = player->inventory_item_count;
    const int text_y_offset = 100;
    const int gap = 30;
    DrawText("Inventory", 10, text_y_offset - gap, 24, WHITE);
    for (int i = 0; i < c; i++) {
        switch (player->inventory[i]) {
        default: {
            printf("Default\n");
            break;
        }
        case ITEM_NOTHING: {
            printf("Error: ITEM_NOTHING present in player inventory.");
            break;
        }
        case ITEM_SPILLEDCUP: {
            DrawText("SpilledCup", 10, text_y_offset + (i * gap), 24, WHITE);
            break;
        }
        case ITEM_STICK: {
            DrawText("Stick", 10, text_y_offset + (i * gap), 24, WHITE);
            break;
        }
        case ITEM_APPLE: {
            DrawText("Apple", 10, text_y_offset + (i * gap), 24, WHITE);
            break;
        }
        }
    }
}

void set_gamestate(GameStateInfo *gsi, enum GameState state) {
    printf("Setting gamestate %i ---> %i.\n", gsi->game_state, state);
    gsi->game_state = state;
    gsi->init = false;
}

void create_entity_instance(EntityData* entity_data, Entity ent) {
    ent.original_position = ent.position;
    entity_data->entities[entity_data->entity_counter] = ent;
    printf("Created entity at index %i.", entity_data->entity_counter);
    entity_data->entity_counter++;
    printf(" Instance counter is now %i\n", entity_data->entity_counter);
}

void remove_entity(const int index, EntityData* entity_data) {
    if (index >= entity_data->entity_counter) {
        printf("Warning: Tried to delete entity index %i, instance counter: %i\n", index, entity_data->entity_counter);
        return;
    }
    for (int i = 0; i < entity_data->entity_counter; i++) {
        entity_data->entities[i] = entity_data->entities[i + 1];
    }
    entity_data->entity_counter--;
    printf("Deleted entity index %i, %i instances left on map.\n", index, entity_data->entity_counter);
}

void generate_enchanted_groves_dungeon_texture(const MapData* map_data, RenderTexture2D* dungeon_texture/*DungeonTexture* dungeon_texture*/) {
	Texture2D floor_texture = LOAD_FOREST_GRASS_TILES_TEXTURE();
	Texture2D terrain_texture = LOAD_FOREST_TERRAIN_TEXTURE();
    Texture2D active_floor_texture = LOAD_FOREST_ACTIVE_GRASS();

	UnloadRenderTexture(*dungeon_texture);
	*dungeon_texture = LoadRenderTexture(map_data->cols * TILE_SIZE, map_data->rows * TILE_SIZE);

    // render to floor layer
	BeginTextureMode(*dungeon_texture);
    {
        // generate floor layer
        for (int row = 0; row < map_data->rows; row++) {
            for (int col = 0; col < map_data->cols; col++) {
                switch (map_data->tiles[col][row]) {
                case TILE_WALL:
                case TILE_CORRIDOR:
                case TILE_ROOM_ENTRANCE:
                case TILE_FLOOR: {
                    DrawTextureRec(
                        floor_texture,
                        (Rectangle) {TILE_SIZE* GetRandomValue(0, 12), 0, TILE_SIZE, -TILE_SIZE},
                        (Vector2) {col* TILE_SIZE, ((map_data->rows - row - 1) * TILE_SIZE)}, 
                        WHITE);
                    bool floor_grass = GetRandomValue(0, 13) == 0;
                    if (floor_grass) {
                        /*DrawTextureRec(
                            active_floor_texture,
                            (Rectangle) {0, 50, TILE_SIZE, -TILE_SIZE},
                            (Vector2) {col* TILE_SIZE, ((map_data->rows - row - 1) * TILE_SIZE)},
                            WHITE);*/
                    }
                    break;
                }
                default: {
                    break;
                }
                }
            }
        }
        // generate terrain layer
        for (int row = 0; row < map_data->rows; row++) {
            for (int col = 0; col < map_data->cols; col++) {
                switch (map_data->tiles[col][row]) {
                case TILE_WALL:
                    DrawTextureRec(
                        terrain_texture,
                        (Rectangle) {
                        150 * GetRandomValue(0, 3), 0, 150, -150
                    },
                        (Vector2) {
                        col* TILE_SIZE - 25, ((map_data->rows - row - 1) * TILE_SIZE - 25)
                    }, WHITE);
                    break;
                default: {
                    break;
                }
                }
            }
        }
    }
	EndTextureMode();

    UnloadTexture(active_floor_texture);
    UnloadTexture(floor_texture);
    UnloadTexture(terrain_texture);
}

//PathList find_path_between_entities(MapData* map_data, bool cut_corners, Entity* start_entity, Entity* target_entity) {
//    PathList path_list = { .path = { 0 }, .length = 0 };
//    //if (target_entity->state != IDLE) return path_list;
//    Point start_point = { (int)start_entity->original_position.x, (int)start_entity->original_position.y };
//    //Point target_point = { (int)target_entity->position.x, (int)target_entity->position.y };
//    Point target_point = { (int)target_entity->original_position.x, (int)target_entity->original_position.y };
//    aStarSearch(map_data, start_point, target_point, &path_list, cut_corners);
//    return path_list;
//}

PathList find_path_between_entities(MapData* map_data, bool cut_corners, Vector2 start_pos, Vector2 end_pos) {
    PathList path_list = { .path = { 0 }, .length = 0 };
    Point start_point = { (int)roundf(start_pos.x), (int)roundf(start_pos.y) };
    Point target_point = { (int)roundf(end_pos.x), (int)roundf(end_pos.y) };

    aStarSearch(map_data, start_point, target_point, &path_list, cut_corners);
    return path_list;
}

void ai_simple_follow_melee_attack(Entity* ent, Entity* target, EntityData* entity_data, MapData* map_data) {
    //if (ent->is_moving) return;
    //if (target->is_moving) return;
    
    // calculate next move state

    Vector2 target_pos = target->original_position;
    if (target->state == MOVE) {
        target_pos = get_tile_infront_entity(target);
    }
    else if (target->state == IDLE) {
		target_pos = target->position;
    }

    // Find the path between the entity and the target
    PathList path_list = find_path_between_entities(map_data, false, ent->original_position, target_pos);

    // If a path is found, move towards the target, otherwise attack if adjacent
    if (path_list.length >= 1) {
        Point next_p = path_list.path[path_list.length - 1];
        Vector2 next_v = (Vector2){ next_p.x, next_p.y };
        Vector2 movement = Vector2Subtract(next_v, ent->original_position);

		ent->direction = vector_to_direction(movement);

        if (any_entity_exists_on_tile(next_v.x, next_v.y, entity_data, ent, NULL)
            && !entity_exists_on_tile(next_v.x, next_v.y, target)) {
            ent->state = SKIP_TURN;
        }
        else {
            ent->state = MOVE;
        }
    }
    else {
        // If next to target and it's this entity's turn, attack
        Vector2 movement = Vector2Subtract(target_pos, ent->original_position);
        ent->direction = vector_to_direction(movement);
		ent->state = ATTACK_MELEE;
    }
}

//void calculate_next_turn(Entity* ent, const MapData* map_data, const EntityData* entity_data) {
//    Entity* player = &entity_data->entities[0];
//    switch (ent->ent_type) {
//    case ENT_FLY:
//        ai_simple_follow_melee_attack(ent, player, entity_data, map_data);
//        break;
//        // Add more cases for other entity types if needed
//    default:
//        // Default behavior if entity type is not recognized
//        break;
//    }
//}

void entity_think(Entity* ent, Entity* player , MapData* map_data, EntityData* entity_data) {
    // player is usually entity index 0
    if (ent == player) {
        // get player next action
        control_entity(ent, map_data->tiles, &entity_data);
    }
    else {
        switch (ent->ent_type) {
        case ENT_FLY: {
            ai_simple_follow_melee_attack(ent, player, entity_data, map_data);
            break;
        }
        default:
            break;
        }
    }
}

bool entity_finished_turn(Entity* ent) {
    return (ent->state == IDLE && ent->n_turn >= ent->max_turns);
}

bool async_moving_entity_exists(EntityData* entity_data) {
    for (int i = 0; i < entity_data->entity_counter; i++) {
        if (entity_data->entities[i].async_move)
            return true;
    }
    return false;
}

void process_entity_state(Entity* ent) {
    switch (ent->state) {
    case IDLE: {
        // not really supposed to trigger here
        break;
    }
    case SKIP_TURN: {
        //reset_entity_state(ent, true);
        break;
    }
    case MOVE: {
        move_entity_forward(ent);
        break;
    }
    case ATTACK_MELEE: {
        lunge_entity(ent);
        break;
    }
    default:
        break;
    }
}

int main(void/*int argc, char* argv[]*/) {

    GameStateInfo gsi = { GS_INTRO_DUNGEON, false };

    int window_width = 1280;
    int window_height = 720;

    InitWindow(window_width, window_height, "mDungeon");
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    //SetTargetFPS(30);

    SetRandomSeed(1337);

    Camera2D camera = { 0 };
    {
        camera.offset = (Vector2){ window_width / 2, window_height / 2 };
        camera.target = (Vector2){ 0.0f, 0.0f };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
    }

    EntityData entity_data = (EntityData){ 0 };
    Entity* zor = { 0 };
    Entity* fantano = { 0 };
    Entity* cyhar = { 0 };

    // init the entities
	create_entity_instance(&entity_data, (Entity) {
		.ent_type = ENT_ZOR,
		.texture = LOAD_ZOR_TEXTURE(),
		.animation = (Animation){ .n_frames = 20 },
        .health = 100,
        .max_turns = 1
	});
    zor = GET_LAST_ENTITY_REF();

	/*create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_FANTANO,
        .texture = LOAD_FANTANO_TEXTURE(),
		.can_swap_positions = true,
		.animation = (Animation){ .n_frames = 20 },
        .health = 100,
        .can_swap_positions = true
	});
	fantano = GET_LAST_ENTITY_REF();*/

	/*create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_CYHAR,
		.texture = LOAD_CYHAR_TEXTURE(),
		.animation = (Animation){ .n_frames = 20 },
        .health = 100
	});
	cyhar = GET_LAST_ENTITY_REF();*/

    create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_FLY,
        .texture = LOAD_FLY_TEXTURE(),
		.animation = (Animation){ .n_frames = 10 },
        .health = 100,
        .can_swap_positions = false,
        .max_turns = 1
    });
    create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_FLY,
            .texture = LOAD_FLY_TEXTURE(),
            .animation = (Animation){ .n_frames = 10 },
            .health = 100,
            .can_swap_positions = false,
            .max_turns = 1
    });

    ItemData item_data = (ItemData){.items = { 0 }, .item_counter = 0};
    nullify_all_items(&item_data);

    // todo: large texture containing multiple item sprites?
    Texture2D texture_item_spilledcup = LOAD_SPILLEDCUP_TEXTURE();
    Texture2D texture_item_stick = LOAD_STICK_TEXTURE();
    Texture2D texture_item_apple = LOAD_APPLE_TEXTURE();
    
    RenderTexture2D dungeon_texture = { 0 };

    MapData map_data = { 0 };

	/*int async_move_ents[MAX_INSTANCES];
	int async_move_ents_counter = 0;*/
    int cur_turn_entity_index = 0;

    // main loop
    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            window_width = GetScreenWidth();
            window_height = GetScreenHeight();
            camera.offset = (Vector2){ window_width / 2.0f, window_height / 2.0f };
        }

        // handle events
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            camera.offset = Vector2Add(camera.offset, GetMouseDelta());
        }
        float scroll = GetMouseWheelMove();
        if (scroll != 0.0f) {
            camera.zoom += scroll * 0.1f;
            if (camera.zoom <= 0.1f) camera.zoom = 0.1f;
            // printf("Scroll: %2.0f\n", camera.zoom);
        }

        // update game logic
        switch (gsi.game_state) {
        case GS_INTRO_DUNGEON: {
            if (!gsi.init) {

                cur_turn_entity_index = 0;
                map_data = generate_map(DUNGEON_PRESET_BASIC);
                generate_enchanted_groves_dungeon_texture(&map_data, &dungeon_texture);

                nullify_all_items(&item_data);
                spawn_items(ITEM_STICK, &item_data, &map_data, 3, 5);
                spawn_items(ITEM_APPLE, &item_data, &map_data, 2, 5);

                for (int i = 0; i < entity_data.entity_counter; i++) {
                    Entity* ent = &entity_data.entities[i];
                    reset_entity_state(ent, false);
                    set_entity_position(ent, find_random_empty_floor_tile(&map_data, &item_data, &entity_data));
                }

                printf("Init basic dungeon.\n");
                gsi.init = true;
            }
            if (IsKeyPressed(KEY_R)) {
                set_gamestate(&gsi, GS_ADVANCED_DUNGEON);
            }
            break;
        }
        case GS_ADVANCED_DUNGEON: {
            if (!gsi.init) {
                // reset tilemap texture 
                Texture2D texture_dungeon_floor_tilemap = LOAD_DESERT_TILES_TEXTURE();
                cur_turn_entity_index = 0;
                // reset dungeon floor texture
                UnloadRenderTexture(dungeon_texture);

                map_data = generate_map(DUNGEON_PRESET_ADVANCED);
                dungeon_texture = LoadRenderTexture(map_data.cols * TILE_SIZE, map_data.rows * TILE_SIZE);

                //for (int row = 0; row < map_data.rows; row++) {
                BeginTextureMode(dungeon_texture);
                for (int row = 0; row < map_data.rows; row++) {
                    for (int col = 0; col < map_data.cols; col++) {
                        switch (map_data.tiles[col][row]) {
                        case TILE_FLOOR:
                        case TILE_CORRIDOR:
                        case TILE_ROOM_ENTRANCE: {
                            DrawTextureRec(
                                texture_dungeon_floor_tilemap,
                                (Rectangle) {
                                TILE_SIZE* GetRandomValue(0, 8), 0, TILE_SIZE, TILE_SIZE
                            },
                                (Vector2) {
                                col* TILE_SIZE, ((map_data.rows - row - 1) * TILE_SIZE)
                            }, WHITE);
                            break;
                        }
                        default: {
                            break;
                        }
                        }
                    }
                }
                EndTextureMode();

                UnloadTexture(texture_dungeon_floor_tilemap);

                // items
                nullify_all_items(&item_data);
                spawn_items(ITEM_SPILLEDCUP, &item_data, &map_data, 1, 3);
                spawn_items(ITEM_STICK, &item_data, &map_data, 4, 8);
                spawn_items(ITEM_APPLE, &item_data, &map_data, 2, 5);
                for (int i = 0; i < entity_data.entity_counter; i++) {
                    Entity* ent = &entity_data.entities[i];
                    reset_entity_state(ent, false);
                    set_entity_position(ent, find_random_empty_floor_tile(&map_data, &item_data, &entity_data));
                }
                printf("Init advanced dungeon.\n");
                gsi.init = true;
            }
            if (IsKeyPressed(KEY_R)) {
                set_gamestate(&gsi, GS_INTRO_DUNGEON);
            }
            break;
        }
        default: {
            printf("Gamestate: %i\n", gsi.game_state);

            printf("Error: default case for gamestate?\n");
            break;
        }
        }
        if (IsKeyPressed(KEY_E) && zor->inventory_item_count > 0) {
            drop_item(zor->inventory_item_count - 1, zor, &item_data);
        }

        // ================================================================== //

        //printf("Cur ent turn: %i\n", cur_turn_entity_index);

        // process entities who are async moving
        if (async_moving_entity_exists(&entity_data)) {

            for (int i = 0; i < entity_data.entity_counter; i++) {
                Entity* ent = &entity_data.entities[i];

                if (!ent->async_move) continue;

				if (entity_finished_turn(ent)) {
					// when an async entity has finished their turn
					cur_turn_entity_index++;
					ent->n_turn = 0;
					if (cur_turn_entity_index >= entity_data.entity_counter) {
						cur_turn_entity_index = 0;
					}
					ent->async_move = false;
				}
				else {
					// if not, rethink turn and process it
					if (ent->state == IDLE || ent->state == SKIP_TURN)
						entity_think(ent, zor, &map_data, &entity_data);
					process_entity_state(ent);
				}
            }
        }
        else {
            // no async moving entities exist currently

            Entity* this_ent = &entity_data.entities[cur_turn_entity_index];

            entity_think(this_ent, zor, &map_data, &entity_data);

            // check if async entities should exist for this turn
            // and log them
            if (this_ent->state == MOVE) {
                for (int i = cur_turn_entity_index + 1; i < entity_data.entity_counter; i++) {
                    Entity* ent = &entity_data.entities[i];
					entity_think(ent, zor, &map_data, &entity_data);
                    if (ent->state != MOVE && ent->state != SKIP_TURN) {
                        break;
                    }
                    this_ent->async_move = true;
                    ent->async_move = true;
                }
            }

            // if there are none, process individual entity as normal
            if (!async_moving_entity_exists(&entity_data)) {

				Entity* this_ent = &entity_data.entities[cur_turn_entity_index];

                entity_think(this_ent, zor, &map_data, &entity_data);

                process_entity_state(this_ent);

				// entity finished turn?
				if (entity_finished_turn(this_ent)) {
					cur_turn_entity_index++;
					this_ent->n_turn = 0;
					if (cur_turn_entity_index >= entity_data.entity_counter) {
						cur_turn_entity_index = 0;
					}
				}
            }
            else {
                
            }
        }

        // ================================================================== //

		for (int i = 0; i < entity_data.entity_counter; i++) {
            Entity* ent = &entity_data.entities[i];
            update_animation_state(ent);
        }

		Vector2 cam_target = { 0 };
		/*if (zor->state == ATTACK_MELEE) {
            cam_target = Vector2Multiply(zor->original_position, (Vector2) { TILE_SIZE, TILE_SIZE });
        }
        else {*/
			cam_target = Vector2Multiply(zor->position, (Vector2) { TILE_SIZE, TILE_SIZE });
        //}
        cam_target.x += ((TILE_SIZE - SPRITE_SIZE) / 2) + (SPRITE_SIZE / 2);
        cam_target.y += ((TILE_SIZE - SPRITE_SIZE) / 2) + (SPRITE_SIZE / 4);
        camera.target = cam_target;

        scan_items_for_pickup(&item_data, zor);
        //scan_items_for_pickup(&item_data, fantano);

		// echo map layout to console
		if (IsKeyPressed(KEY_V)) {
			for (int row = 0; row < map_data.rows; row++) {
				for (int col = 0; col < map_data.cols; col++) {
					switch (map_data.tiles[col][row]) {
					case TILE_WALL: {
						printf("~ ");
						break;
					}
					case TILE_CORRIDOR:
						printf("C ");
					case TILE_FLOOR: {

						//if (inList(&path, (Node){col, row})) {
						/*if (isInPathList(&pathList, (Point) { col, row })) {
							printf("P ");
						}*/
						//else
							printf("X ");
						break;
					}
					case TILE_INVALID: {
						break;
					}
					case TILE_ROOM_ENTRANCE: {
						break;
					}
					default: {
						printf("Error (RENDER): Unknown tile type found on map. (%i, %i) = %i\n", col, row, map_data.tiles[col][row]);
						// printf("? ");
						break;
					}
					}
				}
				printf("\n");
			}
		}

        // render
        {

            // do rendering
            BeginDrawing();
            {
                ClearBackground(BLACK);

                DrawLine(window_width / 2, 0, window_width / 2, window_height, GREEN_SEMI_TRANSPARENT);
                DrawLine(0, window_height / 2, window_width, window_height / 2, GREEN_SEMI_TRANSPARENT);

                BeginMode2D(camera);
                {
                    // render map
					DrawTexture(dungeon_texture.texture, 0, 0, WHITE);
                    for (int row = 0; row < map_data.rows; row++) {
                        for (int col = 0; col < map_data.cols; col++) {
							Color clr = LIGHTGREEN;
                            if (IsKeyDown(KEY_SPACE) 
                                //&& map_data.tiles[(int)zor->position.x][(int)zor->position.y] == map_data.tiles[col][row]) {
                                && map_data.tiles[col][row] != TILE_WALL) {
                                DrawRectangleLines(
                                    col * TILE_SIZE,
                                    row * TILE_SIZE,
                                    TILE_SIZE,
                                    TILE_SIZE,
                                    BLACK_SEMI_TRANSPARENT);
                            }
                        }
                    }

                    // render items
                    for (int i = 0; i < item_data.item_counter; i++) {
                        switch (item_data.items[i].type) {
                        case ITEM_NOTHING: {
                            printf("Error: Tried to render ITEM_NOTHING.");
                            break;
                        }
                        case ITEM_SPILLEDCUP: {
                            DrawTextureRec(texture_item_spilledcup, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(item_data.items[i].position), WHITE);
                            break;
                        }
                        case ITEM_STICK: {
                            DrawTextureRec(texture_item_stick, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(item_data.items[i].position), WHITE);
                            break;
                        }
                        case ITEM_APPLE: {
                            DrawTextureRec(texture_item_apple, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(item_data.items[i].position), WHITE);
                            break;
                        }
                        default: {
                            printf("Idk\n");
                            break;
                        }
                        }
                    }

                    // y sort render entities
                    {
                        bool rendered[MAX_INSTANCES] = { false };
                        for (int i = 0; i < entity_data.entity_counter; i++) {
                            int lowest_y = 99999;
                            int index_to_render = -1;

                            for (int e = 0; e < entity_data.entity_counter; e++) {
                                Entity* ent = &entity_data.entities[e];
                                Vector2 text_pos = position_to_grid_position(ent->position);
                                char txt[8];
                                sprintf_s(txt, 2, "%i", e);
                                DrawText(txt, text_pos.x, text_pos.y, 24, WHITE);
                                int y_pos = ent->position.y * TILE_SIZE/* + (TILE_SIZE / 2)*/;
                                //DrawRectangle(x_pos, y_pos, 50, 50, RED);
                                if (!rendered[e] && y_pos < lowest_y) {
                                    lowest_y = y_pos;
                                    index_to_render = e;
                                }
                            }

                            if (index_to_render != -1) {
                                render_entity(&entity_data.entities[index_to_render]);
                                rendered[index_to_render] = true;
                            }
                        }
                    }

                }
                EndMode2D();
                DrawFPS(10, 5);
				render_player_inventory(zor);
            }
            EndDrawing();
        }
    }

    // delete item textures
    {
        UnloadTexture(texture_item_spilledcup);
        UnloadTexture(texture_item_stick);
        UnloadTexture(texture_item_apple);
    }
    // delete entity textures
    {
        for (int i = 0; i < entity_data.entity_counter; i++) {
            UnloadTexture(entity_data.entities[i].texture);
        }
	}
    
    CloseWindow();
    return 0;
}
