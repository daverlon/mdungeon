/*
    Philosophy:
    Code the game. Nothing more.

    "just type the code for those entities." - Jon Blow
    https://youtu.be/4oky64qN5WI?t=415
*/

#include <stdio.h>
// #include <string.h>

#include "raylib.h"
#include "raymath.h"

#include "dungeon.h"
#include "dungeon_presets.h"

#include "pathfinding.h"
// extern MapData generate_dungeon();

void print_vector2(Vector2 vec) {
    printf("Vector2: (%.2f, %.2f)\n", vec.x, vec.y);
}

#define BLACK_SEMI_TRANSPARENT (Color){0, 0, 0, 128}
#define WHITE_SEMI_TRANSPARENT (Color){255, 255, 255, 64}
#define GREEN_SEMI_TRANSPARENT (Color){0, 255, 50, 32}
#define LIGHTBLUE (Color){50, 100, 200, 150}
#define LIGHTGREEN (Color){50, 200, 100, 250}
#define LIGHTYELLOW (Color){100, 100, 50, 100}

const int world_width = 32 * 14;

#define SPRITE_SIZE 256.0f

// #define TILE_SIZE 128
#define TILE_SIZE 100
// #define TILE_SIZE 256

#define GRID_MOVESPEED 4.0f
#define POSITION_THRESHOLD 0.05f

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

#define MAX_INSTANCES 32
#define INVENTORY_SIZE 32

#define DEFAULT_FRAME_COUNT 20

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

enum EntityState {
    IDLE,
    MOVE,
    ATTACK_MELEE
};

enum ItemType {
    ITEM_NOTHING,
    ITEM_SPILLEDCUP,
    ITEM_STICK,
    ITEM_APPLE
};

//
// level which the game should be running
// all dungeons/levels should be present here
// navigate gamestate using switch statement
//
enum GameState {
    GS_DEFAULT,
    GS_INTRO_DUNGEON,
    GS_ADVANCED_DUNGEON
};

typedef struct {
    enum GameState game_state;
    bool init;
} GameStateInfo;

typedef struct {
    int y_offset;
    int cur_frame;
    float cur_frame_time;
    float max_frame_time;
    int n_frames;
} Animation;

enum EntityType {
    ENT_ZOR,
    ENT_FANTANO,
    ENT_CYHAR,
    ENT_FLY
};

typedef struct {
    enum EntityType ent_type;
    Vector2 position; // grid coordinate position
    Texture2D texture;
    Animation animation;
    enum EntityState state;
    enum Direction direction;
    bool is_moving;
    Vector2 target_position;
    enum ItemType inventory[32];
    int inventory_item_count;
    bool prevent_pickup;
    Vector2 original_position;
    float lunge_progress;
    bool can_swap_positions;
    int health;
} Entity;

typedef struct {
    Entity entities[MAX_INSTANCES];
    int entity_counter;
} EntityData;

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

// move entity 1 square forward (in the direction they're facing)
void move_entity_forward(Entity* en) {
    // move entity grid position
    // (when entity is moving)
    if (!en->is_moving) return;

    const Vector2 movement = direction_to_vector2(en->direction);

    float max_move_distance = GetFrameTime() * GRID_MOVESPEED;

    float distance_to_target = Vector2Distance(en->position, en->target_position);

    if (max_move_distance > distance_to_target) {
        max_move_distance = distance_to_target;
    }

    if (max_move_distance >= distance_to_target) {
        en->position = en->target_position;
        en->is_moving = false;
        en->state = IDLE;
    }
    else {
        en->position.x += movement.x * max_move_distance;
        en->position.y += movement.y * max_move_distance;
    }
}

void control_entity(Entity* en, const enum TileType tiles[MAX_COLS][MAX_ROWS]) {

    // if no key is held, ensure that isMoving is set to false
    if (en->state != IDLE) return;

    bool should_move = false;

    if (IsKeyDown(KEY_S)) {
        if (IsKeyDown(KEY_A)) {
            en->direction = DOWNLEFT;
        }
        else if (IsKeyDown(KEY_D)) {
            en->direction = DOWNRIGHT;
        }
        else {
            en->direction = DOWN;
        }
        should_move = true;
    }

    if (IsKeyDown(KEY_W)) {
        if (IsKeyDown(KEY_A)) {
            en->direction = UPLEFT;
        }
        else if (IsKeyDown(KEY_D)) {
            en->direction = UPRIGHT;
        }
        else {
            en->direction = UP;
        }
        should_move = true;
    }

    if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) {
        en->direction = LEFT;
        should_move = true;
    }
    else if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) {
        en->direction = RIGHT;
        should_move = true;
    }

    if (IsKeyDown(KEY_SPACE) && should_move) {
        should_move = false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !should_move) {
        en->state = ATTACK_MELEE;
        en->animation.cur_frame = 0;
    }

    // this may be a bit scuffed. unecessary extra checks here?
    Vector2 movement = direction_to_vector2(en->direction);
    
    if (!Vector2Equals((movement), (Vector2) { 0 }) && should_move) {
        if (en->prevent_pickup)
            en->prevent_pickup = false;
        // printf("MOVE!\n");
        // printf("X: %2.0f -> %2.0f\n", en->position.x, en->position.x+1.0f);
        // printf("Y: %2.0f -> %2.0f\n", en->position.y, en->position.y+1.0f);
        en->target_position = Vector2Add(en->position, movement);
        // printf("Target: %2.5f, %2.5f\n", en->targetPosition.x, en->targetPosition.y);
        int i_targ_x = (int)en->target_position.x;
        int i_targ_y = (int)en->target_position.y;
        // printf("Target: %i, %i\n", i_targ_x, i_targ_y);

        //
        // todo: fix diagonal tiles
        // idea: walking on flame tiles or other
        //       hazard tiles apply debufs/affects to player
        //       tradeoff for walking on unsafe tile
        //
        // diagonal block
        if (movement.x != 0.0f && movement.y != 0.0f) {
            if (tiles[i_targ_x][(int)en->position.y] == TILE_WALL
            ||  tiles[(int)en->position.x][i_targ_y] == TILE_WALL) {
                //printf("Block diagonal movement\n");
                en->is_moving = false;
                en->state = IDLE;
                return;
            }
        }
        switch (tiles[i_targ_x][i_targ_y]) {
            case TILE_WALL: {
                // printf("Invalid movement.!\n");
                en->is_moving = false;
                en->state = IDLE;
                return;
                break;
            }
            case TILE_ROOM_ENTRANCE:
            case TILE_CORRIDOR:
            case TILE_FLOOR: {
                en->is_moving = true;
                en->state = MOVE;
                break;
            }
            default: {
                printf("Error (MOVEMENT) invalid tile detected?: %i\n", tiles[i_targ_x][i_targ_y]);
                break;
            }
        }
    }
}

bool move_entity_in_direction(Entity* en, enum Direction dir) {
    const Vector2 movement = direction_to_vector2(dir);

    float max_move_distance = GetFrameTime() * GRID_MOVESPEED;

    float distance_to_target = Vector2Distance(en->position, movement);

    if (max_move_distance > distance_to_target) {
        max_move_distance = distance_to_target;
    }

    if (max_move_distance >= distance_to_target) {
        en->position = movement;
        en->is_moving = false;
        en->state = IDLE;
        return true;
    }
    else {
        en->position.x += movement.x * max_move_distance;
        en->position.y += movement.y * max_move_distance;
    }
    return false;
}

void move_entity_freely(Entity* en) {
    // move entity (no grid involved)
    if (!en->is_moving) return;

    Vector2 movement = direction_to_vector2(en->direction);

    en->position.x += movement.x * GetFrameTime() * GRID_MOVESPEED;
    en->position.y += movement.y * GetFrameTime() * GRID_MOVESPEED;

    en->is_moving = false;
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

void render_entity(Entity* en) {
    Vector2 grid_position = position_to_grid_position(en->position);
    // printf("[%i,%i] -> [%i,%i]\n", (int)en->position.x, (int)en->position.y, (int)gridPosition.x, (int)gridPosition.y);
    // printf("%i\n", en->animation.yOffset);

    // offset y
    DrawCircle(
        grid_position.x + (SPRITE_SIZE / 2.0f),
        grid_position.y + (SPRITE_SIZE / 2.0f) + (SPRITE_SIZE / 4.0f),
        35.0f,
        BLACK_SEMI_TRANSPARENT
    );
    // DrawRectangleLines(
    //  gridPosition.x, gridPosition.y, SPRITE_SIZE, SPRITE_SIZE, BLACK);
    DrawTextureRec(
        en->texture,
        (Rectangle) {
        en->animation.cur_frame* SPRITE_SIZE,
            en->animation.y_offset + (en->direction * SPRITE_SIZE),
            SPRITE_SIZE,
            SPRITE_SIZE
    },
        grid_position,
            WHITE
            );
}

void lunge_entity(Entity* en) {
    if (en->state != ATTACK_MELEE) {
        return;
    }

    if (Vector2Equals(en->original_position, (Vector2) { 0, 0 })) {
        en->original_position = en->position;
    }

    const Vector2 movement = direction_to_vector2(en->direction);

    float lunge_distance = 1.0f;
    float lunge_speed = 2.5f;
    float lunge_distance_this_frame = lunge_speed * GetFrameTime();

    if (en->lunge_progress < lunge_distance / 2) {
        en->position = Vector2Add(en->position, Vector2Scale(movement, lunge_distance_this_frame));
    }
    else {
        en->position = Vector2Subtract(en->position, Vector2Scale(movement, lunge_distance_this_frame));
    }

    en->lunge_progress += lunge_distance_this_frame;

    if (en->lunge_progress >= lunge_distance) {
        en->is_moving = false;
        en->lunge_progress = 0.0f;
        en->state = IDLE;
        en->position = en->original_position;
        en->original_position = (Vector2){ 0, 0 };
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
				lunge_entity(ent);
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
				lunge_entity(ent);
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
            lunge_entity(ent);
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
				lunge_entity(ent);
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

bool item_exists_on_tile(int col, int row, const ItemData* item_data) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < item_data->item_counter; i++) {
        if (Vector2Equals(item_data->items[i].position, pos)) {
            return true;
        }
    }
    return false;
}

bool entity_exists_on_tile(int col, int row, const EntityData* entity_data) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < entity_data->entity_counter; i++) {
        if (Vector2Equals(entity_data->entities[i].position, pos)) {
            return true;
        }
    }
    return false;
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

        if (entity_exists_on_tile(col, row, entity_data))
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
    if (entity->is_moving) {
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

PathList find_path_between_entities(MapData* map_data, Entity* start_entity, Entity* target_entity) {
    PathList path_list = { .path = { 0 }, .length = 0 };
    Point start_point = { (int)start_entity->position.x, (int)start_entity->position.y };
    Point target_point = { (int)target_entity->position.x, (int)target_entity->position.y };
    aStarSearch(map_data, start_point, target_point, &path_list, false);
    return path_list;
}

void ai_simple_follow_melee_attack(Entity* ent, Entity* target, MapData* map_data) {
    if (ent->is_moving) return;
    if (target->is_moving) return;

    PathList path_list = find_path_between_entities(map_data, ent, target);
    
    if (path_list.length >= 1) {
        Point next_p = path_list.path[path_list.length-1];
        Vector2 next_v = (Vector2){ next_p.x, next_p.y };
        Vector2 movement = Vector2Subtract(next_v, ent->position);
        ent->direction = vector_to_direction(movement);
    }
    else {
        // next to target
        Vector2 movement = Vector2Subtract(target->position, ent->position);
        if (ent->state == IDLE && target->state == IDLE)
			ent->direction = vector_to_direction(movement);
        ent->state = ATTACK_MELEE;
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
        .health = 100
	});
    zor = GET_LAST_ENTITY_REF();

	create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_FANTANO,
        .texture = LOAD_FANTANO_TEXTURE(),
		.can_swap_positions = true,
		.animation = (Animation){ .n_frames = 20 },
        .health = 100
	});
	fantano = GET_LAST_ENTITY_REF();

	create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_CYHAR,
		.texture = LOAD_CYHAR_TEXTURE(),
		.animation = (Animation){ .n_frames = 20 },
        .health = 100
	});
	cyhar = GET_LAST_ENTITY_REF();

    create_entity_instance(&entity_data, (Entity) {
        .ent_type = ENT_FLY,
        .texture = LOAD_FLY_TEXTURE(),
		.animation = (Animation){ .n_frames = 10 },
        .health = 100
    });

    ItemData item_data = (ItemData){.items = { 0 }, .item_counter = 0};
    nullify_all_items(&item_data);

    // todo: large texture containing multiple item sprites?
    Texture2D texture_item_spilledcup = LOAD_SPILLEDCUP_TEXTURE();
    Texture2D texture_item_stick = LOAD_STICK_TEXTURE();
    Texture2D texture_item_apple = LOAD_APPLE_TEXTURE();
    
    RenderTexture2D dungeon_texture = { 0 };

    MapData map_data = { 0 };

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

                map_data = generate_map(DUNGEON_PRESET_BASIC);
                generate_enchanted_groves_dungeon_texture(&map_data, &dungeon_texture);

                nullify_all_items(&item_data);
                spawn_items(ITEM_STICK, &item_data, &map_data, 3, 5);
                spawn_items(ITEM_APPLE, &item_data, &map_data, 2, 5);

                zor->is_moving = false;
                zor->state = IDLE;
                for (int i = 0; i < entity_data.entity_counter; i++) {
                    entity_data.entities[i].position = find_random_empty_floor_tile(&map_data, &item_data, &entity_data);
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
                zor->is_moving = false;
                zor->state = IDLE;
                zor->position = find_random_empty_floor_tile(&map_data, &item_data, &entity_data);
                fantano->position = find_random_empty_floor_tile(&map_data, &item_data, &entity_data);
                cyhar->position = find_random_empty_floor_tile(&map_data, &item_data, &entity_data);
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
        control_entity(zor, map_data.tiles);
        if (zor->is_moving) {
            move_entity_forward(zor);
        }
        //GET_LAST_ENTITY_REF()->state = ATTACK_MELEE;
		 // perhaps move_entities (once there is ai)
         //move_entity_freely(zor);

        if (IsKeyPressed(KEY_E) && zor->inventory_item_count > 0) {
            drop_item(zor->inventory_item_count - 1, zor, &item_data);
        }

        for (int i = 0; i < entity_data.entity_counter; i++) {
            Entity* ent = &entity_data.entities[i];
            update_animation_state(ent);

            /*if (ent->ent_type == ENT_FLY)
                ai_simple_follow_melee_attack(ent, zor, &map_data);*/

        }
		//ai_simple_follow_melee_attack(fantano, zor, & map_data);

		/*PathList pathList = { .path = { 0 }, .length = 0 };
		aStarSearch(
			&map_data,
			(Point) {(int)GET_LAST_ENTITY_REF()->position.x, (int)GET_LAST_ENTITY_REF()->position.y },
			(Point) {(int)zor->position.x, (int)zor->position.y },
			& pathList,
			false
		);*/
        ai_simple_follow_melee_attack(GET_LAST_ENTITY_REF(), zor, &map_data);


		Vector2 cam_target = { 0 };
		if (zor->state == ATTACK_MELEE) {
            cam_target = Vector2Multiply(zor->original_position, (Vector2) { TILE_SIZE, TILE_SIZE });
        }
        else {
			cam_target = Vector2Multiply(zor->position, (Vector2) { TILE_SIZE, TILE_SIZE });
        }
        cam_target.x += ((TILE_SIZE - SPRITE_SIZE) / 2) + (SPRITE_SIZE / 2);
        cam_target.y += ((TILE_SIZE - SPRITE_SIZE) / 2) + (SPRITE_SIZE / 4);
        camera.target = cam_target;

        scan_items_for_pickup(&item_data, zor);
        scan_items_for_pickup(&item_data, fantano);

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
                            if (IsKeyDown(KEY_SPACE) && map_data.tiles[(int)zor->position.x][(int)zor->position.y] == map_data.tiles[col][row]) {
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
