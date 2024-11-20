/*
    Philosophy:
    Code the game. Nothing more.

    "just type the code for those entities." - Jon Blow
    https://youtu.be/4oky64qN5WI?t=415
*/

#include <stdio.h>
#include <stdlib.h>
// #include <string.h>

#include "main.h"

#include "utils.h"

#include "dungeon.h"
#include "pathfinding.h"
#include "entity_defs.h"

#include "systems/entity_thinking.h"
#include "systems/animation.h"
const int world_width = 32 * 14;
// mobs


#define GET_LAST_ENTITY_REF() (&entity_data->entities[entity_data->entity_counter-1])

// #define NPC_MAX_INVENTORY_SIZE 4

//#define VIEW_DISTANCE 5.0f

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

        // check adjacent tiles for corridor (may cause blocking)
        if (map_data->tiles[col + 1][row].type == TILE_CORRIDOR) continue; // right
        if (map_data->tiles[col][row + 1].type == TILE_CORRIDOR) continue; // down
        if (map_data->tiles[col - 1][row].type == TILE_CORRIDOR) continue; // left
        if (map_data->tiles[col][row - 1].type == TILE_CORRIDOR) continue; // up

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
        item_data->items[i] = (Item){ ITEM_NOTHING, ITEMCAT_NOTHING, (Vector2) { 0, 0 }, 100 };
    }
    item_data->item_counter = 0;
    printf("Set every item to ITEM_NOTHING\n");
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

Vector2 find_random_empty_floor_tile(const MapData* map_data, const ItemData* item_data, const EntityData* entity_data) {
    int col = -1;
    int row = -1;

    // while (1) {
    //     // choose tile
    //     col = GetRandomValue(0, MAX_COLS);
    //     row = GetRandomValue(0, MAX_ROWS);

    //     if (map_data->tiles[col + 1][row].type == TILE_CORRIDOR) continue; // right
    //     if (map_data->tiles[col][row + 1].type == TILE_CORRIDOR) continue; // down
    //     if (map_data->tiles[col - 1][row].type == TILE_CORRIDOR) continue; // left
    //     if (map_data->tiles[col][row - 1].type == TILE_CORRIDOR) continue; // up

    //     // check if tile is TILE_FLOOR
    //     if (map_data->tiles[col][row].type != TILE_FLOOR)
    //         continue;

    //     // check if item exists on tile
    //     if (item_exists_on_tile(col, row, item_data, NULL))
    //         continue;

    //     if (any_entity_exists_on_tile(col, row, entity_data, NULL, NULL))
    //         continue;

    //     // todo:
    //     // check if entity exists on tile

    //     break;
    // }
    return (Vector2) { col, row };
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

void scan_items_for_pickup(ItemData* item_data, Entity* entity) {
    if (!entity->can_pickup) return;
    //if (!entity->prevent_pickup) {
        // entity can pickup item
    // for (int i = 0; i < item_data->item_counter; i++) {
    //     Item* item = &item_data->items[i];
    //     // item exists on same tile as entity
    //     if (
    //         (entity->state == MOVE && Vector2Equals(item->position, get_tile_infront_entity(entity))
    //             && (!Vector2Equals(item->position, entity->original_position))
    //             )) {
    //         pickup_item(i, item_data, entity);
    //         break;
    //     }
    // }
    //}
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

    printf("Deleted entity index %i, %i instances left on map.\n", index, entity_data->entity_counter - 1);
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

    // map_data->view_distance = 10.0f;

    // Texture2D floor_texture = LOAD_FOREST_GRASS_TILES_TEXTURE();
    // Texture2D terrain_texture = LOAD_FOREST_TERRAIN_TEXTURE();
    // Texture2D active_floor_texture = LOAD_FOREST_ACTIVE_GRASS();

    // UnloadRenderTexture(*dungeon_texture);
    // *dungeon_texture = LoadRenderTexture(map_data->cols * TILE_SIZE, map_data->rows * TILE_SIZE);

    // // render to floor layer
    // BeginTextureMode(*dungeon_texture);
    // {
    //     // generate floor layer
    //     for (int row = 0; row < map_data->rows; row++) {
    //         for (int col = 0; col < map_data->cols; col++) {
    //             switch (map_data->tiles[col][row].type) {
    //             case TILE_WALL:
    //             case TILE_CORRIDOR:
    //             case TILE_ROOM_ENTRANCE:
    //             case TILE_FLOOR: {
    //                 DrawTextureRec(
    //                     floor_texture,
    //                     (Rectangle) {
    //                     TILE_SIZE* GetRandomValue(0, 12), 0, TILE_SIZE, -TILE_SIZE
    //                 },
    //                     (Vector2) {
    //                     col* TILE_SIZE, ((map_data->rows - row - 1) * TILE_SIZE)
    //                 },
    //                         WHITE);
    //                 bool floor_grass = GetRandomValue(0, 13) == 0;
    //                 if (floor_grass) {
    //                     /*DrawTextureRec(
    //                         active_floor_texture,
    //                         (Rectangle) {0, 50, TILE_SIZE, -TILE_SIZE},
    //                         (Vector2) {col* TILE_SIZE, ((map_data->rows - row - 1) * TILE_SIZE)},
    //                         WHITE);*/
    //                 }
    //                 break;
    //             }
    //             default: {
    //                 break;
    //             }
    //             }
    //         }
    //     }
    //     // generate terrain layer
    //     for (int row = 0; row < map_data->rows; row++) {
    //         for (int col = 0; col < map_data->cols; col++) {
    //             switch (map_data->tiles[col][row].type) {
    //             case TILE_WALL:
    //                 DrawTextureRec(
    //                     terrain_texture,
    //                     (Rectangle) {
    //                     150 * GetRandomValue(0, 3), 0, 150, -150
    //                 },
    //                     (Vector2) {
    //                     col* TILE_SIZE - 25, ((map_data->rows - row - 1) * TILE_SIZE - 25)
    //                 }, WHITE);
    //                 break;
    //             default: {
    //                 break;
    //             }
    //             }
    //         }
    //     }
    // }
    // EndTextureMode();

    // UnloadTexture(active_floor_texture);
    // UnloadTexture(floor_texture);
    // UnloadTexture(terrain_texture);
}

//PathList find_sync_path_to_entity(MapData* map_data, Vector2 start_pos, Vector2 end_pos) {
//    PathList path_list = { .path = { 0 }, .length = 0 };
//    Point start_point = { (int)roundf(start_pos.x), (int)roundf(start_pos.y) };
//    Point target_point = { (int)roundf(end_pos.x), (int)roundf(end_pos.y) };
//
//    aStarSearch(map_data, start_point, target_point, &path_list, true, false, NULL, NULL);
//    return path_list;
//}


//PathList find_path_around_entities(MapData* map_data, Vector2 start_pos, Vector2 end_pos, EntityData* entity_data, Entity* ignore) {
//    PathList path_list = { .path = { 0 }, .length = 0 };
//    Point start_point = { (int)roundf(start_pos.x), (int)roundf(start_pos.y) };
//    Point target_point = { (int)roundf(end_pos.x), (int)roundf(end_pos.y) };
//
//    aStarSearch(map_data, start_point, target_point, &path_list, false, true, entity_data, ignore);
//    return path_list;
//}


// from entity position -> map position
//
//extern void aStarSearch(MapData* map, PathList* path_list, Point src, Point dest, Entity src_ent, bool cut_world_corners);



void run_enchanted_groves_dungeon(GameStateInfo* gsi, EntityData* entity_data, ItemData* item_data, MapData* map_data) {
    static Entity* zor;
    static Entity* fantan;
    
    // in it
	if (!gsi->init) {
		gsi->cur_turn_entity_index = 0;
        gsi->cur_turn = 0;
        snprintf(gsi->area_name, 17, "Enchanted Groves");
		*map_data = generate_map(DUNGEON_PRESET_BASIC);
		generate_enchanted_groves_dungeon_texture(map_data, &map_data->dungeon_texture);

		// init the entities
		nullify_all_entities(entity_data);

		create_entity_instance(entity_data, default_ent_zor());

		zor = GET_LAST_ENTITY_REF();
		// zor->max_turns = 2;
		printf("Zor cur room: %i\n", zor->cur_room);

		// create_entity_instance(entity_data, create_fantano_entity());
		// fantan = GET_LAST_ENTITY_REF();

		//zor->atk = ;
		for (int i = 0; i < 28; i++) {
			create_entity_instance(entity_data, create_fly_entity());
			//GET_LAST_ENTITY_REF()->max_turns = GetRandomValue(1, 2);
		}

		// init items
		nullify_all_items(item_data);
		spawn_items_on_random_tiles((Item) { ITEM_STICK, ITEMCAT_WEAPON, (Vector2){0.0f, 0.0f}, 100 },  item_data,  map_data, 5, 8);
		spawn_items_on_random_tiles((Item) { ITEM_APPLE, ITEMCAT_CONSUMABLE, (Vector2){0.0f, 0.0f}, 100 },  item_data,  map_data, 4, 7);
		spawn_items_on_random_tiles((Item) { ITEM_SPILLEDCUP, ITEMCAT_CONSUMABLE, (Vector2){0.0f, 0.0f}, 100 },  item_data,  map_data, 2, 4);

		// for (int i = 0; i < entity_data->entity_counter; i++) {
		// 	Entity* ent = &entity_data->entities[i];
		// 	reset_entity_state(ent, false);
		// 	set_entity_position(ent, find_random_empty_floor_tile(map_data, item_data, entity_data));
		// }

		zor->cur_room = get_room_id_at_position((int)zor->original_position.x, (int)zor->original_position.y, map_data);
		// ai_fantano_teleport_to_same_room_as_player_and_defend(fantan, zor, entity_data, map_data, item_data);

		printf("Init basic dungeon.\n");

		gsi->init = true;
	}

	if (IsKeyPressed(KEY_R)) {
		gsi->cur_turn = 0;
		set_gamestate(gsi, GS_INTRO_DUNGEON);
		entity_data->entity_counter = 0;
	}

    zor->hp = zor->max_hp;
}

int main(void/*int argc, char* argv[]*/) {

	GameStateInfo gsi = { 
        .window_width = 1280,
        .window_height = 720,
        .game_state = GS_INTRO_DUNGEON,
	    .init = false,
	    .cur_turn_entity_index = 0,
		.cur_turn = 0,
		.grid_mouse_position = { 0 }
	};

  
    InitWindow(gsi.window_width, gsi.window_height, "mDungeon");
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    //SetWindowState(FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(144);

    SetRandomSeed(100);

    Font fonts[1] = { 
        LoadFontEx("dependencies/res/fonts/YanoneKaffeesatz-Regular.ttf", 144, NULL, 0) 
    };
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_POINT);

    Camera2D camera = { 0 };
    {
        camera.offset = (Vector2){ gsi.window_width / 2, gsi.window_height / 2 };
        camera.target = (Vector2){ 0.0f, 0.0f };
        camera.rotation = 0.0f;
        camera.zoom = 1.3f;
    }

    EntityData entity_data = (EntityData){ 0 };
    nullify_all_entities(&entity_data);

    ItemData item_data = (ItemData){ .items = { 0 }, .item_counter = 0 };
    nullify_all_items(&item_data);

    // todo: large texture containing multiple item sprites?


    //RenderTexture2D dungeon_texture = { 0 };

    MapData map_data = { 0 };

    FullState fullState = (FullState){
        gsi,
        map_data,
        entity_data,
        item_data
    };

    // main loop
    while (!WindowShouldClose()) {



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

        // check for ents who are dead
        // for (int i = 0; i < entity_data.entity_counter; i++) {
        //     Entity* ent = &entity_data.entities[i];
        //     // if (ent == NULL) continue;

        //     Vector2 act = get_active_position(ent);
        //     ent->cur_room = get_room_id_at_position(act.x, act.y, &map_data);

        //     if (ent->hp <= 0) {
        //         drop_random_item(ent, &item_data);
        //         increment_entity_fade(ent);

        //         if (ent->faded && i != 0) {
        //             remove_entity(i, &entity_data);
        //         }
        //         else {
        //             i++;
        //         }
        //     }
        //     else {
        //         // ent alive
        //         i++;
        //     }
        // }

        // update game logic
        switch (gsi.game_state) {
        case GS_INTRO_DUNGEON: {
            run_enchanted_groves_dungeon(&gsi, &entity_data, &item_data, &map_data);
            process_entity_turn_queue(&gsi, &entity_data, &item_data, &map_data);
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

         // ================================================================== //

         // debug statements
         //printf("\nEntity turn id: %i\n", cur_turn_entity_index);
         /*for (int i =24 0; i < entity_data.entity_counter; i++) {
             Entity* ent = &entity_data.entities[i];
             printf("Entity %i state %i async %i ||| turn %i/%i ||| maxframe %.5f\n", i, ent->state, (int)ent->sync_move, ent->n_turn, ent->max_turns, ent->animation.max_frame_time);
         }*/

        Entity* zor = &entity_data.entities[0];


        // ================================================================== //
        // update entity animations

        // for (int i = 0; i < entity_data.entity_counter; i++) {
        //     Entity* ent = &entity_data.entities[i];
        //     if (is_entity_dead(ent)) continue;

        //     update_animation_state(ent);
        //     update_animation(&ent->animation);

        //     //scan_items_for_pickup(&item_data, ent);
        // }

        // ================================================================== //
        // update camera and render, etc 

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
                        /*if (map_data.tiles[col][row].reserved)
                            printf("R ");
                        else*/
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

        // condition to reset dungeon when player dead
        if (zor->hp <= 0) {
            set_gamestate(&gsi, GS_INTRO_DUNGEON);
            continue;
        }

        // update damage popup timer
        // decrement_entity_notif_timer(&entity_data);

        
    }

    // ================================================================== //
    // unload textures and data, etc

    UnloadRenderTexture(map_data.dungeon_texture);
    

    UnloadFont(fonts[0]);

    CloseWindow();
    return 0;
}
