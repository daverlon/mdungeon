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

#include "entity_defs.h"

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
// mobs

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

#define FOG_AMOUNT 0.3f
//#define VIEW_DISTANCE 5.0f

void rotate_smooth(enum Direction target, enum Direction* dir) {
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
    // check if item already exists at current position 

    for (int i = 0; i < item_data->item_counter; i++) {
        if (Vector2Equals(entity->original_position, item_data->items[i].position)) {
            printf("Tried to drop item on top of existing item.\n");
            return;
        }
    }
    item_data->items[item_data->item_counter] = entity->inventory[index];
    item_data->items[item_data->item_counter].position = entity->original_position;
    item_data->item_counter++;
    delete_item_from_entity_inventory(index, entity);
    //entity->prevent_pickup = true;
}

void drop_random_item(Entity* entity, ItemData* item_data) {

    if (!(entity->inventory_item_count > 0)) return;

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

    int index = GetRandomValue(0, entity->inventory_item_count-1);
    item_data->items[item_data->item_counter] = entity->inventory[index];
    item_data->item_counter++;
    delete_item_from_entity_inventory(index, entity);
    //entity->prevent_pickup = true;
}

Vector2 get_active_position(Entity* ent) {
    switch (ent->state) {
    case MOVE:
        //return ent->position;
        //return get_tile_infront_entity(ent);
        return ent->position;
        break;
    /*case ATTACK_MELEE:
        return ent->original_position;*/
        break;
    default: 
        break;
    }
    return ent->original_position;
}


bool any_entity_exists_on_tile(int col, int row, const EntityData* entity_data, Entity* ignore, int* out) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < entity_data->entity_counter; i++) {
        Entity* ent = &entity_data->entities[i];

        if (ignore != NULL && ent == ignore) continue;

        Vector2 ent_target_position = get_tile_infront_entity(ent);
        if (
            (ent->state == MOVE && Vector2Equals(ent_target_position, pos))
            || (ent->state != MOVE && Vector2Equals(ent->original_position, pos))
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
      (ent->state == MOVE && Vector2Equals(ent_target_position, pos) && Vector2Equals(ent->position, pos))
    || (ent->state != MOVE && Vector2Equals(ent->original_position, pos))
    ) {
        return true;
    }
    return false;
}

void reset_entity_state(Entity* ent, bool use_turn) {
    ent->state = IDLE;
    //ent->original_position = ent->position;
    if (use_turn) {
        ent->n_turn++;
        ent->attack_damage_given = false;
    }
}

void move_entity_forward(Entity* ent) {
    //if (ent->state != MOVE) return;

    const Vector2 movement = direction_to_vector2(ent->direction);
    //const Vector2 normalized_movement = Vector2Normalize(movement); // Normalize the movement vector
    const Vector2 targ = Vector2Add(ent->original_position, movement);

    float distance_to_target = Vector2Distance(ent->position, targ);

    //float max_move_distance = GetFrameTime() * GRID_MOVESPEED;
    float max_move_distance = GetFrameTime() * (IsKeyDown(KEY_LEFT_SHIFT) ? GRID_MOVESPEED * 2 : GRID_MOVESPEED);

    if (max_move_distance >= distance_to_target) {
        ent->position = targ;
        ent->original_position = ent->position;
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

void control_entity(Entity* ent, MapData* map_data, EntityData* entity_data, ItemData* item_data, Vector2 grid_mouse_position) {

    // if no key is held, ensure that isMoving is set to false
    if (ent->state != IDLE) return;

	if (IsKeyPressed(KEY_E) && ent->inventory_item_count > 0) {
		drop_item(ent->inventory_item_count - 1, ent, item_data);
	}

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

    if (IsKeyDown(KEY_SPACE)) {
        should_move = false;

		Vector2 mouse_grid_pos_dir = Vector2Clamp(
			Vector2Subtract(grid_mouse_position, ent->original_position),
			(Vector2) {
			-1.0f, -1.0f
		},
		(Vector2) {
			1.0f, 1.0f
		});
        if (Vector2Length(GetMouseDelta()) != 0.0f && Vector2Length(mouse_grid_pos_dir) != 0.0f)
            ent->direction = vector_to_direction(mouse_grid_pos_dir);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !should_move) {
        ent->state = ATTACK_MELEE;
        ent->animation.cur_frame = 0;
    }

    // this may be a bit scuffed. unecessary extra checks here?
    Vector2 movement = direction_to_vector2(ent->direction);

    if (should_move) {
        /*if (ent->prevent_pickup)
            ent->prevent_pickup = false;*/

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
            if (map_data->tiles[i_targ_x][(int)ent->position.y].type == TILE_WALL
                || map_data->tiles[(int)ent->position.x][i_targ_y].type == TILE_WALL) {
                reset_entity_state(ent, false);
                return;
            }
        }

        // entity block/swap
        int swapi = -1;
        if (any_entity_exists_on_tile(i_targ_x, i_targ_y, entity_data, ent, &swapi)) {
            //printf("Tried to walk on tile with entity\n");
            if (swapi != -1)
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

        switch (map_data->tiles[i_targ_x][i_targ_y].type) {
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
            printf("Error (MOVEMENT) invalid tile detected?: %i\n", map_data->tiles[i_targ_x][i_targ_y].type);
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
    /*DrawRectangle(
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
    );*/

	// DrawRectangleLines(
    //  gridPosition.x, gridPosition.y, SPRITE_SIZE, SPRITE_SIZE, BLACK);
    if (ent->fade_timer > 0.0f) {
        DrawTextureRec(
            ent->texture,
            (Rectangle) {
            ent->animation.cur_frame* SPRITE_SIZE,
                ent->animation.y_offset + (ent->direction * SPRITE_SIZE),
                SPRITE_SIZE,
                SPRITE_SIZE
        },
            grid_position,
                Fade(WHITE, 1.0f - (ent->fade_timer / FADE_DURATION))
                );
    }
    else {
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


		// hp bar width
		int hp_bar_x = grid_position.x + TILE_SIZE / 1.3 + 2;
		int hp_bar_y = grid_position.y + SPRITE_SIZE / 1.2;
		int hp_bar_height = 10;
		int hp_bar_width = TILE_SIZE;
		int hp_cur_width = ((float)ent->hp / (float)ent->max_hp) * (float)hp_bar_width;
		DrawRectangle(hp_bar_x, hp_bar_y, hp_bar_width, hp_bar_height, BLACK);
		DrawRectangle(hp_bar_x, hp_bar_y, hp_cur_width, hp_bar_height, GREEN);

    }
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

void switch_to_idle_y_offset(Entity* ent) {
	if (ent->animation.y_offset != 0) {
		ent->cur_move_anim_extra_frame++;
	}
	if (ent->cur_move_anim_extra_frame >= MOVE_ANIMATION_EXTRA_FRAMES) {
		//ent->animation.max_frame_time = 0.02f;
		ent->animation.y_offset = 0;
	}
}

void update_animation_state(Entity* ent) {
    switch (ent->ent_type) {
    case ENT_ZOR: {
        switch (ent->state) {
        case IDLE: {

            ent->animation.max_frame_time = 0.020f;
            switch_to_idle_y_offset(ent);
            
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
            //ent->cur_move_anim_extra_frame = 0;
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
            switch_to_idle_y_offset(ent);

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
            switch_to_idle_y_offset(ent);

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
            switch_to_idle_y_offset(ent);

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

	if (ent->state != IDLE) {
		ent->cur_move_anim_extra_frame = 0;
	}
}

Vector2 find_random_empty_floor_tile(const MapData* map_data, const ItemData* item_data, const EntityData* entity_data) {
    int col = -1;
    int row = -1;

    while (1) {
        // choose tile
        col = GetRandomValue(0, MAX_COLS);
        row = GetRandomValue(0, MAX_ROWS);

        // check if tile is TILE_FLOOR
        if (map_data->tiles[col][row].type != TILE_FLOOR)
            continue;

        // check if item exists on tile
        if (item_exists_on_tile(col, row, item_data))
            continue;

        if (any_entity_exists_on_tile(col, row, entity_data, NULL, NULL))
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

void spawn_items_on_random_tiles(Item item, ItemData* item_data, const MapData* map_data, int min, int max) {
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
        if (map_data->tiles[col][row].type != TILE_FLOOR)
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
        item.position = (Vector2){ col, row };
        item.hp = 100;
        create_item_instance(item, item_data);
        item_counter++;
    }
}

void nullify_all_items(ItemData* item_data) {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        item_data->items[i] = (Item){ ITEM_NOTHING, ITEMCAT_NOTHING, (Vector2) { 0, 0 } };
    }
    item_data->item_counter = 0;
    printf("Set every item to ITEM_NOTHING\n");
}

//void calculate_stats_at_level(EntityStats *stats) {
//    // Calculate HP and ATK based on base stats and growth rates
//    *hp =  + (level - 1) * HP_PER_LEVEL;
//    *atk = BASE_ATK + (level - 1) * 3;
//}

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
    if (entity->inventory_item_count < entity->inventory_size) {
        entity->inventory[entity->inventory_item_count] = item_data->items[index];
        entity->inventory_item_count++;
		delete_item(index, item_data);
    }
    // else: "inventory full"?
}

void scan_items_for_pickup(ItemData* item_data, Entity* entity) {
    if (!entity->can_pickup) return;
    //if (!entity->prevent_pickup) {
        // entity can pickup item
        for (int i = 0; i < item_data->item_counter; i++) {
            Item* item = &item_data->items[i];
            // item exists on same tile as entity
            if (
                (entity->state == MOVE && Vector2Equals(item->position, get_tile_infront_entity(entity))
             && (!Vector2Equals(item->position, entity->original_position))
			)) {
                pickup_item(i, item_data, entity);
                break;
            }
        }
    //}
}

void get_item_name(enum ItemType it, char* buf) {
    switch (it) {
    default:
        sprintf_s(buf, 9, "empty"); 
        break;
    case ITEM_APPLE:
        sprintf_s(buf, 6, "apple"); 
        break;
    case ITEM_STICK:
        sprintf_s(buf, 6, "stick"); 
        break;
    case ITEM_SPILLEDCUP:
        sprintf_s(buf, 12, "spilled cup");
        break;
    }
}

void render_ui(int window_width, int window_height, Entity* player, Font* fonts) {


    int y = 30;

    //y += 25;

    // hp bar
    {
        int w = 300;
        /*int h = (float)window_height * 0.0166666667;*/
        int h = 12;
        //int x = window_width / 2 - w / 2;
        int x = window_width - w - 30;

        Rectangle hp_bar = (Rectangle){ x, y, w, h };
        int hp_hp = ((float)player->hp / (float)player->max_hp) * hp_bar.width;

        //DrawRectangleGradientV(x, y, w, h, RED, (Color){64, 0, 0, 255});
        DrawRectangleGradientV(x, y, w, h, (Color) { 64, 0, 0, 255 }, RED);
        DrawRectangleGradientV(x, y, hp_hp, h, GREEN, DARKGREEN);
        DrawRectangleRoundedLines(hp_bar, 0.5f, 1.0f, 2, BLACK);

        char txt[10];
        sprintf_s(txt, 10, "%i/%i", player->hp, player->max_hp);

        int yy = y - 1;
        int xx = x + 2;
        DrawTextEx(fonts[0], txt, (Vector2) { xx +2, yy +2 }, 24.0f, 1.0f, Fade(BLACK, 0.7f));
        DrawTextEx(fonts[0], txt, (Vector2) { xx, yy }, 24.0f, 1.0f, WHITE);
    }

    y += 25;

    // item hp bar
    {
        int w = 210;
        int h = 12;
        int x = window_width - w - 30;

        int item_i = player->equipped_item_index;
        Item* equipped_item = &player->inventory[item_i];

        Rectangle hp_bar = (Rectangle){ x, y, w, h };
        int hp_hp = ((float)equipped_item->hp / (float)player->max_hp) * hp_bar.width;

		//DrawRectangleGradientV(x, y, w, h, GRAY, (Color){30, 30, 30, 255});
		DrawRectangleGradientV(x, y, w, h, (Color) { 30, 30, 30, 255 }, GRAY);
		DrawRectangleGradientV(x, y, hp_hp, h, BLUE, (Color) { 0, 0, 202, 255 });
		DrawRectangleRoundedLines(hp_bar, 0.5f, 1.0f, 2, BLACK);

		char txt[16];
        get_item_name(player->inventory[item_i].type, txt);

        Vector2 fs = MeasureTextEx(fonts[0], txt, 24.0f, 1.0f);
        //x = window_width - 30 - fs.x - 3;

        DrawTextEx(fonts[0], txt, (Vector2) { x + 4, y + 2}, 24.0f, 1.0f, Fade(BLACK, 0.7f));
        DrawTextEx(fonts[0], txt, (Vector2) { x+2, y }, 24.0f, 1.0f, WHITE);

    }

}

void render_player_inventory(Entity* player) {
    // for debug/development purposes
    const int c = player->inventory_item_count;
    const int text_y_offset = 100;
    const int gap = 30;
    DrawText("Inventory", 10, text_y_offset - gap, 24, WHITE);
    for (int i = 0; i < c; i++) {
        switch (player->inventory[i].type) {
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

void set_gamestate(GameStateInfo* gsi, enum GameState state) {
    printf("Setting gamestate %i ---> %i.\n", gsi->game_state, state);
    gsi->game_state = state;
    gsi->init = false;
}

void create_entity_instance(EntityData* entity_data, Entity ent) {
    ent.original_position = ent.position;
    ent.direction = GetRandomValue(0, 7);
    entity_data->entities[entity_data->entity_counter] = ent;
    ent.cur_move_anim_extra_frame = MOVE_ANIMATION_EXTRA_FRAMES;
    printf("Created entity at index %i.", entity_data->entity_counter);
    entity_data->entity_counter++;
    printf(" Instance counter is now %i\n", entity_data->entity_counter);
}

void remove_entity(const int index, EntityData* entity_data) {
    if (index < 0 || index >= entity_data->entity_counter) {
        printf("Warning: Tried to delete entity with invalid index %i, instance counter: %i\n", index, entity_data->entity_counter);
        return;
    }

    // Shift entities down to fill the gap left by the removed entity
    for (int i = index; i < entity_data->entity_counter - 1; i++) {
        entity_data->entities[i] = entity_data->entities[i + 1];
    }

    // Decrement the counter since one entity has been removed
    entity_data->entity_counter--;

    printf("Deleted entity index %i, %i instances left on map.\n", index, entity_data->entity_counter-1);
}

void nullify_all_entities(EntityData* entity_data) {
    for (int i = 0; i < entity_data->entity_counter; i++) {
        UnloadTexture(entity_data->entities[i].texture);
        entity_data->entities[i].ent_type = ENT_NOTHING;
    }
    entity_data->entity_counter = 0;
}

void increment_entity_fade(Entity* ent) {
    ent->fade_timer += GetFrameTime();

	if (ent->fade_timer >= FADE_DURATION) {
        ent->faded = true;
        //ent->opacity = 0.0f;  // Ensure opacity is set to 0 when fading completes
	}
	else {
		// Linear interpolation of opacity based on fading progress
        //ent->opacity = 1.0f - (ent->fade_timer / FADE_DURATION);
	}
}

void generate_enchanted_groves_dungeon_texture(MapData* map_data, RenderTexture2D* dungeon_texture/*DungeonTexture* dungeon_texture*/) {

    map_data->view_distance = 10.0f;

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
                switch (map_data->tiles[col][row].type) {
                case TILE_WALL:
                case TILE_CORRIDOR:
                case TILE_ROOM_ENTRANCE:
                case TILE_FLOOR: {
                    DrawTextureRec(
                        floor_texture,
                        (Rectangle) {
                        TILE_SIZE* GetRandomValue(0, 12), 0, TILE_SIZE, -TILE_SIZE
                    },
                        (Vector2) {
                        col* TILE_SIZE, ((map_data->rows - row - 1) * TILE_SIZE)
                    },
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
                switch (map_data->tiles[col][row].type) {
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

bool is_entity_visible(Entity* from, Entity* to, MapData* map_data, bool look_into_corridors) {
    //printf("from %i - to %i\n", from->cur_room, to->cur_room);

    // check if target is moving into the same room
    // dont think it does anything?
    if (to->state == MOVE) {
        int moving_to_i = get_room_id_at_position(
            get_tile_infront_entity(to).x,
            get_tile_infront_entity(to).y,
            map_data);
        if (moving_to_i != -1 && moving_to_i == from->cur_room) {
            return true;
        }
    }

    if (look_into_corridors) {
        if (to->cur_room == -1) {
            if (from->cur_room != -1) {
                float distance = Vector2DistanceSqr(get_active_position(from), get_active_position(to));
                //printf("Distance: %2.5f\n", distance);
                if (distance <= map_data->view_distance)
                    return true;
            }
        }
    }

    if (from->cur_room != -1) {
        if (to->cur_room != from->cur_room) {

            if (to->state != MOVE)
				return false;
            else {
                if (get_room_id_at_position(
                    get_tile_infront_entity(to).x, 
                    get_tile_infront_entity(to).y, 
                    map_data) == from->cur_room) {
						return true;
                }
                else {
                    return false;
                }
            }
        }
    }
    // in corridor
    else {
		//float distance = Vector2DistanceSqr(get_active_position(from), get_active_position(to));
		float distance = Vector2DistanceSqr(get_active_position(from), get_active_position(to));
		//printf("Distance: %2.5f\n", distance);
        if (distance > map_data->view_distance) 
            return false;
    }
    return true;
}

bool is_item_visible(Entity* from, Item* item, MapData* map_data) {
    int item_room = get_room_id_at_position(item->position.x, item->position.y, map_data);
    // entity in a room
    if (from->cur_room != -1) {
        // match room with item
        if (item_room != from->cur_room) {
            return false;
        }
    }
    else {
        // entity not in a room

        //float distance = Vector2DistanceSqr(get_active_position(from), get_active_position(to));
        float distance = Vector2DistanceSqr(get_active_position(from), item->position);
        //printf("Distance: %2.5f\n", distance);
        if (distance > map_data->view_distance)
            return false;
    }
    return true;
}

void ai_simple_follow_melee_attack(Entity* ent, Entity* target, EntityData* entity_data, MapData* map_data) {
    //if (ent->is_moving) return;
    //if (target->is_moving) return;

    // calculate next move state
    if (!ent->found_target) {
        if (is_entity_visible(ent, target, map_data, false)) {
            if (!ent->found_target) {
                ent->found_target = true;
                //printf("Found target\n");
            }
        }
    }
    // still no target found
    if (!ent->found_target) {
        ent->state = SKIP_TURN;
        return;
    }

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

        if (ent->state == IDLE)
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
        if (ent->state == IDLE)
			ent->direction = vector_to_direction(movement);
        ent->state = ATTACK_MELEE;
    }
}

void entity_think(Entity* ent, Entity* player, MapData* map_data, EntityData* entity_data, ItemData* item_data, Vector2 grid_mouse_position) {
    // player is usually entity index 0
    if (ent == player) {
        // get player next action
        control_entity(ent, map_data, entity_data, item_data, grid_mouse_position);
    }
    else {
        switch (ent->ent_type) {
        case ENT_ZOR:
        case ENT_FLY: {
            ai_simple_follow_melee_attack(ent, player, entity_data, map_data);
            break;
        }
        default:
            break;
        }
    }
}

bool is_entity_dead(Entity* ent) {
    return (ent->hp <= 0);
}

bool entity_finished_turn(Entity* ent) {
    if (is_entity_dead(ent)) {
        return ent->faded;
    }
    else {
        return (ent->state == IDLE && ent->n_turn >= ent->max_turns);
    }
}

bool sync_moving_entity_exists(EntityData* entity_data) {
    for (int i = 0; i < entity_data->entity_counter; i++) {
        if (entity_data->entities[i].sync_move)
            return true;
    }
    return false;
}

int value_variation(float value, int percentage) {
    int v = GetRandomValue(-percentage, percentage);
    float vf = 1.0 + (v / 100.0);

    float fval = roundf(value);
    fval *= vf;
    return (int)roundf(value);
}

int get_melee_attack_damage(Entity* ent, int *self_damage) {
    float damage = (float)ent->atk;

    // todo: ui etc for equipped item
    if (ent->inventory_item_count <= 0) return (int)roundf(damage);

    switch (ent->inventory[ent->equipped_item_index].type) {
    case ITEM_NOTHING: {
        if (GetRandomValue(0, 9))
            damage *= 1.7f;
        break;
    }
    case ITEM_APPLE: {
        damage += 5.0f;
        *self_damage = 100;
        break;
    }
    case ITEM_SPILLEDCUP: {
        damage += 4.0f;
        *self_damage = 100;
        break;
    }
	case ITEM_STICK: {
        // crit
		damage *= 2.5f;
        if (GetRandomValue(0, 2) == 0)
            damage *= 1.7f;

        *self_damage = 50;
		break;
	}
	default:
		break;
	}
    
    damage = value_variation(damage, 5);
    //printf("Damage: %2.5f\n", fdamage);

    return damage;
}

void apply_damage(Entity* to, Entity* from, int amount, bool change_direction) {
    printf("Damage: %i\n", amount);
    to->hp -= amount;
    
    // spin the damaged entity around to look at the other ent

    Vector2 direction = Vector2Clamp(Vector2Subtract(from->original_position, to->original_position),
        (Vector2) {
        -1.0f, -1.0f
    },
        (Vector2) {
        1.0f, 1.0f
    });

    /*if (change_direction && to->state == IDLE && !Vector2Equals(direction, (Vector2){0.0f, 0.0f}))
		to->direction = vector_to_direction(direction);*/

    from->attack_damage_given = true;
    to->found_target = true;
}

void process_attack(Entity* ent, EntityData* entity_data) {
    enum EntityState attack_state = ent->state;

    switch (attack_state) {
    case ATTACK_MELEE: {
        
        Vector2 tile = get_tile_infront_entity(ent);

        int id = 0;
        if (any_entity_exists_on_tile(tile.x, tile.y, entity_data, NULL, &id)) {
            if (!ent->attack_damage_given) {
                if (ent->animation.cur_frame > 7) {
                    int self_damage = 0;
					int damage = get_melee_attack_damage(ent, &self_damage);
                    printf("Damage: %i ||| Self Damage: %i\n", damage, self_damage);
                    ent->inventory[ent->equipped_item_index].hp -= self_damage;
                    apply_damage(&entity_data->entities[id], ent, damage, true);
				}
            }
        }
        break;
    }
    default: {
        //printf("Error: invalid attack state given.\n");
        break;
    }
    }
    if (ent->inventory[ent->equipped_item_index].type != ITEM_NOTHING && ent->inventory[ent->equipped_item_index].hp <= 0) {
        delete_item_from_entity_inventory(ent->equipped_item_index, ent);
    }
}

void process_entity_state(Entity* ent, EntityData* entity_data) {
    switch (ent->state) {
    case IDLE: {
        // not really supposed to trigger here
        break;
    }
    case SKIP_TURN: {
        reset_entity_state(ent, true);
        break;
    }
    case MOVE: {
        move_entity_forward(ent);
        break;
    }
    case ATTACK_MELEE: {
        lunge_entity(ent);
        process_attack(ent, entity_data);
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
    //SetWindowState(FLAG_VSYNC_HINT);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    //SetWindowState(FLAG_WINDOW_MAXIMIZED);
    //SetTargetFPS(30);
    SetTargetFPS(144);

    SetRandomSeed(99);

    Font fonts[1] = { 0 };
    fonts[0] = LoadFontEx("res/fonts/YanoneKaffeesatz-Regular.ttf", 144, NULL, NULL);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_POINT);

    Camera2D camera = { 0 };
    {
        camera.offset = (Vector2){ window_width / 2, window_height / 2 };
        camera.target = (Vector2){ 0.0f, 0.0f };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
    }
    Vector2 grid_mouse_position = { 0 };

    EntityData entity_data = (EntityData){ 0 };
    Entity* zor = { 0 };
    Entity* fantano = { 0 };
    Entity* cyhar = { 0 };


    ItemData item_data = (ItemData){ .items = { 0 }, .item_counter = 0 };
    nullify_all_items(&item_data);

    // todo: large texture containing multiple item sprites?
    Texture2D texture_item_spilledcup = LOAD_SPILLEDCUP_TEXTURE();
    Texture2D texture_item_stick = LOAD_STICK_TEXTURE();
    Texture2D texture_item_apple = LOAD_APPLE_TEXTURE();

    RenderTexture2D dungeon_texture = { 0 };
    RenderTexture2D fog_texture = LoadRenderTexture(window_width, window_height);

    MapData map_data = { 0 };

    int cur_turn_entity_index = 0;
    //int cur_room = -1;

    // main loop
    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            window_width = GetScreenWidth();
            window_height = GetScreenHeight();
            camera.offset = (Vector2){ window_width / 2.0f, window_height / 2.0f };

            UnloadRenderTexture(fog_texture);
            fog_texture = LoadRenderTexture(window_width, window_height);
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

				// init the entities
                nullify_all_entities(&entity_data);
                create_entity_instance(&entity_data, default_ent_zor());
				zor = GET_LAST_ENTITY_REF();
                zor->max_turns = 1;
                zor->atk = 20;
                for (int i = 0; i < 8; i++)
                    create_entity_instance(&entity_data, create_fly_entity());
                        
                // init items
				nullify_all_items(&item_data);
                spawn_items_on_random_tiles((Item) { ITEM_STICK, ITEMCAT_WEAPON}, & item_data, & map_data, 5, 8);
                spawn_items_on_random_tiles((Item) { ITEM_APPLE, ITEMCAT_CONSUMABLE }, &item_data, &map_data, 4, 7);
                spawn_items_on_random_tiles((Item) { ITEM_SPILLEDCUP, ITEMCAT_CONSUMABLE }, &item_data, &map_data, 2, 4);

                for (int i = 0; i < entity_data.entity_counter; i++) {
                    Entity* ent = &entity_data.entities[i];
                    reset_entity_state(ent, false);
                    set_entity_position(ent, find_random_empty_floor_tile(&map_data, &item_data, &entity_data));
                }

                printf("Init basic dungeon.\n");
                gsi.init = true;
            }
            if (IsKeyPressed(KEY_R)) {
                set_gamestate(&gsi, GS_INTRO_DUNGEON);
                entity_data.entity_counter = 0;
            }
            break;
        }
        default: {
            printf("Gamestate: %i\n", gsi.game_state);

            printf("Error: default case for gamestate?\n");
            break;
        }
        }

        // start of update ------------------------
        //cur_room = get_room_id_at_position(zor->position.x, zor->position.y, &map_data);
        //printf("Cur room: %i\n", zor->cur_room);

		// check for ents who are dead
        for (int i = 0; i < entity_data.entity_counter; ) {
            Entity* ent = &entity_data.entities[i];
            
            Vector2 act = get_active_position(ent);
            ent->cur_room = get_room_id_at_position(act.x, act.y, &map_data);

            if (ent->hp <= 0) {
                drop_random_item(ent, &item_data);
                increment_entity_fade(ent);
                if (ent->faded) {
                    remove_entity(i, &entity_data);
                }
                else {
                    i++;
                }
            }
            else {
                i++;
            }
        }

       /* if (player_is_dead) {
            set_gamestate(&gsi, GS_INTRO_DUNGEON);
            continue;
        }*/

        // ================================================================== //

        // debug statements
        //printf("\nEntity turn id: %i\n", cur_turn_entity_index);
        /*for (int i =24 0; i < entity_data.entity_counter; i++) {
            Entity* ent = &entity_data.entities[i];
            printf("Entity %i state %i async %i ||| turn %i/%i ||| maxframe %.5f\n", i, ent->state, (int)ent->sync_move, ent->n_turn, ent->max_turns, ent->animation.max_frame_time);
        }*/


        // process entities who are in sync moving
		if (sync_moving_entity_exists(&entity_data)) {

			for (int i = 0; i < entity_data.entity_counter; i++) {
				Entity* ent = &entity_data.entities[i];

				if (!ent->sync_move)
					continue;

                if (is_entity_dead(ent))
                    ent->state = IDLE;

				if (entity_finished_turn(ent)) {
					// when an in sync entity has finished their turn
					cur_turn_entity_index++;
					ent->n_turn = 0;
					if (cur_turn_entity_index >= entity_data.entity_counter) {
						cur_turn_entity_index = 0;
					}
					ent->sync_move = false;
				}
				else {
					// if not, rethink turn and process it
					if (ent->state == IDLE || ent->state == SKIP_TURN) {
						entity_think(ent, zor, &map_data, &entity_data, &item_data, grid_mouse_position);
					}
					process_entity_state(ent, &entity_data);
				}
			}
		}
		else {
			// no sync moving entities exist currently

			Entity* this_ent = &entity_data.entities[cur_turn_entity_index];

			entity_think(this_ent, zor, &map_data, &entity_data, &item_data, grid_mouse_position);

			// check if sync entities should exist for this turn
			// and log them
			if (this_ent->state == MOVE) {
				for (int i = cur_turn_entity_index + 1; i < entity_data.entity_counter; i++) {
					Entity* ent = &entity_data.entities[i];

					entity_think(ent, zor, &map_data, &entity_data, &item_data, grid_mouse_position);
					if (ent->state != MOVE && ent->state != SKIP_TURN) {
						reset_entity_state(ent, false);
						break;
					}
					this_ent->sync_move = true;
					ent->sync_move = true;
				}
			}


			// if there are none, process individual entity as normal
			if (!sync_moving_entity_exists(&entity_data)) {

				Entity* this_ent = &entity_data.entities[cur_turn_entity_index];

				entity_think(this_ent, zor, &map_data, &entity_data, &item_data, grid_mouse_position);

                if (is_entity_dead(this_ent))
                    this_ent->state = IDLE;

				process_entity_state(this_ent, &entity_data);

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
            if (is_entity_dead(ent)) continue;

			update_animation_state(ent);
            update_animation(&ent->animation);

			scan_items_for_pickup(&item_data, ent);
        }

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


        // echo map layout to console
        if (IsKeyPressed(KEY_V)) {
            for (int row = 0; row < map_data.rows; row++) {
                for (int col = 0; col < map_data.cols; col++) {
                    switch (map_data.tiles[col][row].type) {
                    case TILE_WALL: {
                        printf("~ ");
                        break;
                    }
                    case TILE_CORRIDOR:
                        printf("C ");
                        break;
                    case TILE_FLOOR: {
                        printf("X ");
                        break;
                    }
                    case TILE_INVALID: {
                        break;
                    }
                    case TILE_ROOM_ENTRANCE: {
                        printf("E ");
                        break;
                    }
                    default: {
                        printf("Error (RENDER): Unknown tile type found on map. (%i, %i) = %i\n", col, row, map_data.tiles[col][row].type);
                        // printf("? ");
                        break;
                    }
                    }
                }
                printf("\n");
            }
        }

        if (zor->hp <= 0) {
            set_gamestate(&gsi, GS_INTRO_DUNGEON);
            continue;
        }


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
						if (IsKeyDown(KEY_SPACE) && map_data.tiles[col][row].type != TILE_WALL) {
							Color clr = BLACK_SEMI_TRANSPARENT;

							grid_mouse_position = GetMousePosition();
							grid_mouse_position = GetScreenToWorld2D(grid_mouse_position, camera);
							grid_mouse_position = Vector2Divide(grid_mouse_position, (Vector2) { TILE_SIZE, TILE_SIZE });
							grid_mouse_position.x = floorf(grid_mouse_position.x);
							grid_mouse_position.y = floorf(grid_mouse_position.y);
							//print_vector2(grid_mouse_position);

							if (grid_mouse_position.x == col && grid_mouse_position.y == row) {
								clr = YELLOW;
							}

							if (Vector2Equals(get_tile_infront_entity(zor), (Vector2){col, row})) {
								clr = ORANGE;
							}
							
							Vector2 mouse_grid_pos_dir = Vector2Clamp(
								Vector2Subtract(grid_mouse_position, zor->original_position),
								(Vector2) {
								-1.0f, -1.0f
							},
								(Vector2) {
								1.0f, 1.0f
							});
							//print_vector2(mouse_grid_pos_dir);
							
							if (Vector2Equals(Vector2Add(zor->original_position, mouse_grid_pos_dir), (Vector2) { col, row })) {
								clr = RED;
							}

							DrawRectangleLines(
								col * TILE_SIZE,
								row * TILE_SIZE,
								TILE_SIZE,
								TILE_SIZE,
								clr);
					}
					}
				}

				// render items
				for (int i = 0; i < item_data.item_counter; i++) {
                    if (!is_item_visible(zor, &item_data.items[i], &map_data)) continue;

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
			} // end camera rendering
			EndMode2D();

            // render fog texture
            {
                BeginTextureMode(fog_texture);
                BeginBlendMode(BLEND_SUBTRACT_COLORS);
                ClearBackground(Fade(BLACK, FOG_AMOUNT)); // Dim the whole screen

                if (zor->cur_room != -1) {
                    Room* rm = &map_data.rooms[zor->cur_room];

                    // Calculate the room's position and size based on TILE_SIZE
                    Vector2 tl = (Vector2){ rm->x * TILE_SIZE, rm->y * TILE_SIZE };
                    tl.x -= TILE_SIZE / 4;
                    tl.y -= TILE_SIZE / 4;
                    tl = GetWorldToScreen2D(tl, camera);

                    Vector2 br = (Vector2){ (rm->x + rm->cols) * TILE_SIZE, (rm->y + rm->rows) * TILE_SIZE };
                    br = GetWorldToScreen2D(br, camera);
                    br.x += TILE_SIZE / 4;
                    br.y += TILE_SIZE / 4;

                    //Vector2 delta = Vector2Subtract(br, tl);
                    Vector2 delta = (Vector2){
                       br.x - tl.x,
                       br.y - tl.y
                    };
                    tl.y += delta.y;

                    //DrawRectangle(tl.x, window_height - tl.y, delta.x, delta.y, Fade(BLACK, 0.5f));
                    Rectangle light = (Rectangle){ tl.x, window_height - tl.y, delta.x, delta.y };
                    DrawRectangleRounded(light, 0.3f, 0.0f, Fade(BLACK, FOG_AMOUNT));
                }
                else {
                    Vector2 pos = { 0 };

                    if (zor->state != MOVE)
						pos = Vector2Multiply(zor->original_position, (Vector2) { TILE_SIZE, TILE_SIZE });
                    else 
						pos = Vector2Multiply(zor->position, (Vector2) { TILE_SIZE, TILE_SIZE });

                    pos.x += TILE_SIZE / 2;
                    pos.y -= TILE_SIZE / 4;
                    Vector2 screen_pos = GetWorldToScreen2D(pos, camera);

                    float circle_radius = TILE_SIZE * pow(map_data.view_distance, 0.5f) * camera.zoom;

                    DrawCircle(screen_pos.x, screen_pos.y, circle_radius, Fade(BLACK, FOG_AMOUNT));
                }

                EndBlendMode();
                EndTextureMode(); // End drawing to texture

                // Draw the fog texture over the whole screen
                DrawTexture(fog_texture.texture, 0, 0, WHITE);
            }

            BeginMode2D(camera);
			// y sort render entities
			{
				bool rendered[MAX_INSTANCES] = { false };
				for (int i = 0; i < entity_data.entity_counter; i++) {
					int lowest_y = 99999;
					int index_to_render = -1;

					for (int e = 0; e < entity_data.entity_counter; e++) {
						Entity* ent = &entity_data.entities[e];

                        if (e != 0) {
                            if (!is_entity_visible(zor, ent, &map_data, false)) continue;
                        }

						//Vector2 text_pos = position_to_grid_position(ent->position);

					  /*  char txt[8];
						sprintf_s(txt, 2, "%i", e);
						DrawText(txt, text_pos.x, text_pos.y, 24, WHITE);*/


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
            EndMode2D();


			DrawFPS(10, 5);
			render_player_inventory(zor);
            render_ui(window_width, window_height, zor, fonts);

		} // end rendering
        EndDrawing();
    }

    UnloadRenderTexture(dungeon_texture);
    UnloadRenderTexture(fog_texture);

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

    UnloadFont(fonts[0]);

    CloseWindow();
    return 0;
}