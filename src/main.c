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

#define BLACK_SEMI_TRANSPARENT (Color){0, 0, 0, 128}
#define WHITE_SEMI_TRANSPARENT (Color){255, 255, 255, 64}
#define GREEN_SEMI_TRANSPARENT (Color){0, 255, 50, 32}
#define LIGHTBLUE (Color){50, 100, 200, 150}
#define LIGHTGREEN (Color){50, 200, 100, 50}

const int worldWidth = 32 * 14;

#define SPRITE_SIZE 256.0f

// #define TILE_SIZE 128
#define TILE_SIZE 160
// #define TILE_SIZE 256

#define GRID_MOVESPEED 4.0f
#define POSITION_THRESHOLD 0.05f

#define LOAD_ZOR_TEXTURE() (LoadTexture("res/zor/zor_spritesheet.png"))
#define LOAD_FANTANO_TEXTURE() (LoadTexture("res/fantano/fantano_idle.png"))
#define LOAD_CYHAR_TEXTURE() (LoadTexture("res/cyhar/cyhar_idle.png"))

#define LOAD_SPILLEDCUP_TEXTURE() (LoadTexture("res/items/item_spilledcup.png"))
#define LOAD_STICK_TEXTURE() (LoadTexture("res/items/item_stick.png"))
#define LOAD_APPLE_TEXTURE() (LoadTexture("res/items/item_apple.png"))

#define MAX_INSTANCES 32
#define INVENTORY_SIZE 32

#define DEFAULT_FRAME_COUNT 20

// #define NPC_MAX_INVENTORY_SIZE 4

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

enum AnimationState {
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
    enum GameState gameState;
    bool init;
} GameStateInfo;

typedef struct {
    int yOffset;
    int curFrame;
    float curFrameTime;
    float maxFrameTime;
    int nFrames;
} Animation;

typedef struct {
    Vector2 position; // grid coordinate position
    Texture2D texture;
    Animation animation;
    enum AnimationState animationState;
    enum Direction direction;
    bool isMoving;
    Vector2 targetPosition;
    enum ItemType inventory[32];
    int inventoryItemCount;
} Entity;

typedef struct {
    enum ItemType type;
    Vector2 position; // grid coordinate position
    // for player to ignore item once dropped
    // enemies should still be able to pickup the item (perhaps some won't want to though)
    bool preventPickup;  // default 0: (0=can pickup)
} Item;
// ideas:   BasicItem (items with consistent effects)
//          SpecialItem (items that may change? this seems weird..)
//          ItemWear (wear value for each item (such that they are disposable? probably not fun)) 

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

void control_entity(Entity* en, const enum TileType tiles[MAX_COLS][MAX_ROWS]) {

    // if no key is held, ensure that isMoving is set to false
    if (en->isMoving) return;
    if (en->animationState == ATTACK_MELEE) return;
    en->animationState = IDLE;

    bool shouldMove = false;

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
		shouldMove = true;
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
		shouldMove = true;
    }

    if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) {
        en->direction = LEFT;
		shouldMove = true;
    }
    else if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) {
        en->direction = RIGHT;
		shouldMove = true;
    }

    if (IsKeyDown(KEY_SPACE) && shouldMove) {
        shouldMove = false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !shouldMove) {
        en->animationState = ATTACK_MELEE;
        en->animation.curFrame = 0;
    }

    // this may be a bit scuffed. unecessary extra checks here?
    Vector2 movement = direction_to_vector2(en->direction);
    
    if (!Vector2Equals((movement), (Vector2) { 0 }) && shouldMove) {
        // printf("MOVE!\n");
        // printf("X: %2.0f -> %2.0f\n", en->position.x, en->position.x+1.0f);
        // printf("Y: %2.0f -> %2.0f\n", en->position.y, en->position.y+1.0f);
        en->targetPosition = Vector2Add(en->position, movement);
        // printf("Target: %2.5f, %2.5f\n", en->targetPosition.x, en->targetPosition.y);
        int i_targ_x = (int)en->targetPosition.x;
        int i_targ_y = (int)en->targetPosition.y;
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
                en->isMoving = false;
                en->animationState = IDLE;
                return;
            }
        }
        switch (tiles[i_targ_x][i_targ_y]) {
            case TILE_WALL: {
                // printf("Invalid movement.!\n");
                en->isMoving = false;
                en->animationState = IDLE;
                return;
                break;
            }
            case TILE_ROOM_ENTRANCE:
            case TILE_CORRIDOR:
            case TILE_FLOOR: {
                en->isMoving = true;
                en->animationState = MOVE;
                break;
            }
            default: {
                printf("Error (MOVEMENT) invalid tile detected?: %i\n", tiles[i_targ_x][i_targ_y]);
                break;
            }
        }

    }
}

void move_entity(Entity* en) {
    // move entity grid position
    // (when entity is moving)
    if (!en->isMoving) return;

    // printf("%2.5f, %2.5f\n", en->position.x, en->position.y);
    // printf("%2.5f, %2.5f\n", en->position.x, en->position.y);

    const Vector2 movement = direction_to_vector2(en->direction);
    // printf("Cur: %2.5f, %2.5f\n", en->position.x, en->position.y);
    // printf("Tar: %2.5f, %2.5f\n", en->position.x + movement.x, en->position.y + movement.y);

    // linear interpolation
    // en->position.x = Lerp(en->position.x, en->targetPosition.x, t);
    // en->position.y = Lerp(en->position.y, en->targetPosition.y, t);

    // const Vector2 
    // if ()

    en->position.x += movement.x * GetFrameTime() * GRID_MOVESPEED;
    en->position.y += movement.y * GetFrameTime() * GRID_MOVESPEED;

    // Check if the entity has reached the target position
    float distance = Vector2Distance(en->position, en->targetPosition);
    if (distance < POSITION_THRESHOLD) {
        en->position = en->targetPosition;  // Ensure exact position when close
        en->isMoving = false;
        en->animationState = IDLE;
        // printf("Moved entity to X: %2.0f -> %2.0f\n", en->position.x, en->position.x+1.0f);
    }
}

void move_entity_freely(Entity* en) {
    // move entity (no grid involved)
    if (!en->isMoving) return;

    Vector2 movement = direction_to_vector2(en->direction);

    en->position.x += movement.x * GetFrameTime() * GRID_MOVESPEED;
    en->position.y += movement.y * GetFrameTime() * GRID_MOVESPEED;

    en->isMoving = false;
}

void update_animation_frame(Animation* anim) {
    float* ft = &anim->curFrameTime;
    (*ft) += GetFrameTime();
    int* cf = &anim->curFrame;

    if ((*cf) >= anim->nFrames) {
        (*cf) = 0;
    }
    else {
        (*cf)++;
    }
    (*ft) = 0.0f;
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
    Vector2 gridPosition = position_to_grid_position(en->position);
    // printf("[%i,%i] -> [%i,%i]\n", (int)en->position.x, (int)en->position.y, (int)gridPosition.x, (int)gridPosition.y);
    // printf("%i\n", en->animation.yOffset);

    // offset y
    DrawCircle(
        gridPosition.x + (SPRITE_SIZE / 2),
        gridPosition.y + (SPRITE_SIZE / 2) + (SPRITE_SIZE / 4),
        35.0f, 
        BLACK_SEMI_TRANSPARENT
    );
    // DrawRectangleLines(
    //  gridPosition.x, gridPosition.y, SPRITE_SIZE, SPRITE_SIZE, BLACK);
    DrawTextureRec(
        en->texture,
        (Rectangle) {
            en->animation.curFrame * SPRITE_SIZE, 
            en->animation.yOffset + (en->direction * SPRITE_SIZE), 
            SPRITE_SIZE, 
            SPRITE_SIZE
        },
        gridPosition,
        WHITE
    );
}

Entity create_zor_entity_instance(Vector2 pos) {
    Entity en = { 0 };
    en.position = pos;
    en.animation.nFrames = 20;
    return en;
}

void update_zor_animation(Entity* zor) {

    // higher fps seems to speed this up
    switch (zor->animationState) {
    case IDLE: {
        zor->animation.maxFrameTime = 0.0f;
        zor->animation.yOffset = 0;
        break;
    }
    case MOVE: {
        zor->animation.maxFrameTime = 0.02f;
        // zor->animation.yOffset = 2048.0f;
        zor->animation.yOffset = 2048;
        break;
    }
    case ATTACK_MELEE: {
        zor->animation.maxFrameTime = 0.015f;
        zor->animation.yOffset = 2048 + 2048;
        if (zor->animation.curFrame >= zor->animation.nFrames) {
            zor->animationState = IDLE;
        }
        break;
    }
    default: {
        zor->animation.maxFrameTime = 0.0f;
        zor->animation.yOffset = 0;
        break;
    }
    }

    float* ft = &zor->animation.curFrameTime;
    *ft += GetFrameTime();
    int* cf = &zor->animation.curFrame;

    //printf("Max frames: %i\n", zor->animation.nFrames);
    if (*ft >= zor->animation.maxFrameTime) {
        if (*cf >= zor->animation.nFrames) {
            (*cf) = 0;
        }
        else {
            (*cf)++;
        }
        (*ft) = 0.0f;
    }
    //printf("Cur frame: %i\n", *cf);
}

Vector2 find_random_empty_floor_tile(const MapData* mapData) {
    int col = GetRandomValue(0, MAX_COLS);
    int row = GetRandomValue(0, MAX_ROWS);
    while (mapData->tiles[col][row] != TILE_FLOOR) {
        col = GetRandomValue(0, MAX_COLS);
        row = GetRandomValue(0, MAX_ROWS);
    }
    return (Vector2) { col, row };
}

void create_item_instance(Item item, int* counter, Item* items) {
    //printf("Lol: %i\n", *counter);
    if ((*counter) >= MAX_INSTANCES) {
        printf("Cannot spawn item. Maximum items reached.\n");
        return;
    }
    if (item.type == ITEM_NOTHING) {
        printf("Error: Tried to create ITEM_NOTHING.\n");
        return;
    }
    items[*counter] = item;
    (*counter)++;
}

void delete_item(const int index, int* counter, Item* items) {
    if (index >= *counter) {
        printf("Warning: Tried to delete item index: %i, instance counter: %i\n", index, *counter);
        return;
    }

    for (int i = index; i < *counter; i++) {
        items[i] = items[i + 1];
    }
    (*counter)--;
    printf("Deleted item index: %i, %i instances left on map.\n", index, *counter);
}

void spawn_items(enum ITEM_TYPE itemType, int* mapItemCounter, Item* mapItems, const MapData* mapData, int min, int max) {
    if (*mapItemCounter >= MAX_INSTANCES || *mapItemCounter + max >= MAX_INSTANCES) {
        printf("Error: [Spawn Items] mapItemCounter already at maximum instances.\n");
        return;
    }

    int n_items = GetRandomValue(min, max);
    int item_counter = 0;
    while (item_counter < n_items) {
        //for (int i = 0; i < n_spilledcups; i++) {
        int col = GetRandomValue(0, MAX_COLS);
        int row = GetRandomValue(0, MAX_ROWS);
        if (mapData->tiles[col][row] != TILE_FLOOR)
            continue;
        // tile is floor
        bool position_taken = false;
        for (int i = 0; i < n_items; i++) {
            if (Vector2Equals(
                mapItems[i].position,
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
        create_item_instance((Item) { itemType, (Vector2) { col, row }, false }, & (*mapItemCounter), mapItems);
        item_counter++;
    }
}

void nullify_all_items(int* counter, Item* items) {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        items[i] = (Item){ ITEM_NOTHING, (Vector2) { 0, 0 }, true };
    }
    (*counter) = 0;
    printf("Set every item to ITEM_NOTHING\n");
}

// all items should probably not be on 1 texture.
// only certain items can appear at a time
// todo: un-generic this function
// void render_items_on_map(int* counter, Item* items, Texture2D tx) {
    // requires textures 
// }

void pickup_item(const int index, Item* mapItems, Entity* entity) {
    // add item to entity inventory
    // this should be called with delete_item called after
    if (entity->inventoryItemCount < INVENTORY_SIZE) {
        entity->inventory[entity->inventoryItemCount] = mapItems[index].type;
        entity->inventoryItemCount++;
    }
}

void scan_items_for_pickup(int* counter, Item* mapItems, Entity* entity) {
    for (int i = 0; i < *counter; i++) {
        Item* item = &mapItems[i];
        if (Vector2Equals(item->position, entity->position)) {
            if (!item->preventPickup) {
                pickup_item(i, mapItems, entity);
                delete_item(i, counter, mapItems);
                printf("Entity item pickup.\n");
            }
        }
        else {
            if (item->preventPickup) {
                item->preventPickup = false;
            }
        }
    }
}

void delete_item_from_entity_inventory(const int index, Entity* entity) {
    if (index >= entity->inventoryItemCount) {
        printf("Warning: Tried to delete inventory item index: %i, item counter: %i\n", index, entity->inventoryItemCount);
        return;
    }

    for (int i = index; i < entity->inventoryItemCount; i++) {
        entity->inventory[i] = entity->inventory[i + 1];
    }
    entity->inventoryItemCount--;
    printf("Deleted inventory item index: %i, %i items remaining.\n", index, entity->inventoryItemCount);
}

void drop_item(const int index, Entity* entity, int* mapItemCounter, Item* mapItems) {
    if (entity->isMoving) {
        printf("Tried to drop item while entity is moving.\n");
        return;
    }
    // check if item already exists at current position 
    for (int i = 0; i < *mapItemCounter; i++) {
        if (Vector2Equals(entity->position, mapItems[i].position)) {
            printf("Tried to drop item on top of existing item.\n");
            return;
        }
    }
    mapItems[*mapItemCounter] = (Item){ entity->inventory[index], entity->position, true };
    (*mapItemCounter)++;
    delete_item_from_entity_inventory(index, entity);
}

void render_player_inventory(Entity* player) {
    // for debug/development purposes
    const int c = player->inventoryItemCount;
    const int yOffset = 100;
    const int gap = 30;
    DrawText("Inventory", 10, yOffset - gap, 24, WHITE);
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
            DrawText("SpilledCup", 10, yOffset + (i * gap), 24, WHITE);
            break;
        }
        case ITEM_STICK: {
            DrawText("Stick", 10, yOffset + (i * gap), 24, WHITE);
            break;
        }
        case ITEM_APPLE: {
            DrawText("Apple", 10, yOffset + (i * gap), 24, WHITE);
            break;
        }
        }
    }
}

void set_gamestate(GameStateInfo *gsi, enum GameState state) {
    printf("Setting gamestate %i ---> %i.\n", gsi->gameState, state);
    gsi->gameState = state;
    gsi->init = false;
}

int main(void/*int argc, char* argv[]*/) {

    GameStateInfo gsi = { GS_INTRO_DUNGEON, false };

    int window_width = 1280;
    int window_height = 720;

    InitWindow(window_width, window_height, "mDungeon");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Camera2D camera = { 0 };
    {
        camera.offset = (Vector2){ window_width / 2, window_height / 2 };
        camera.target = (Vector2){ 0.0f, 0.0f };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
    }

    Entity fantano = { 0 };
    {
        fantano.texture = LOAD_FANTANO_TEXTURE();
        fantano.position = (Vector2){ 5.0f, 5.0f };
        fantano.animation.nFrames = 20;
	}

    Entity cyhar = { 0 };
    {
        cyhar.texture = LOAD_CYHAR_TEXTURE();
        cyhar.position = (Vector2){ 1.0f, 0.0f };
        cyhar.animation.nFrames = 20;
    }

    Entity zor = create_zor_entity_instance((Vector2) { 7.0f, 7.0f });
    zor.texture = LOAD_ZOR_TEXTURE();

    Item mapItems[MAX_INSTANCES];
    int mapItemCounter = 0;
    nullify_all_items(&mapItemCounter, mapItems);

    // todo: large texture containing multiple item sprites?
    Texture2D spilledCupTx = LOAD_SPILLEDCUP_TEXTURE();
    Texture2D stickTx = LOAD_STICK_TEXTURE();
    Texture2D appleTx = LOAD_APPLE_TEXTURE();

    MapData mapData = { 0 };
    int current_floor = 0; 

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
        int scroll = GetMouseWheelMove();
        if (scroll != 0.0f) {
            camera.zoom += scroll * 0.1f;
            if (camera.zoom <= 0.1f) camera.zoom = 0.1f;
            // printf("Scroll: %2.0f\n", camera.zoom);
        }

        // update game logic
        switch (gsi.gameState) {
			case GS_INTRO_DUNGEON: {
                // init intro dungeon
                if (!gsi.init) {
                    // init intro dungeon
                    nullify_all_items(&mapItemCounter, mapItems);
                    mapData = generate_map(DUNGEON_PRESET_BASIC);
                    spawn_items(
                        ITEM_SPILLEDCUP,
                        &mapItemCounter,
                        mapItems,
                        &mapData,
                        1, 3
                    );
                    spawn_items(
                        ITEM_STICK,
                        &mapItemCounter,
                        mapItems,
                        &mapData,
                        4, 8
                    );
                    spawn_items(
                        ITEM_APPLE,
                        &mapItemCounter,
                        mapItems,
                        &mapData,
                        2, 5
                    );
                    zor.isMoving = false;
                    zor.animationState = IDLE;
                    zor.position = find_random_empty_floor_tile(&mapData);
                    fantano.position = find_random_empty_floor_tile(&mapData);
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
                    nullify_all_items(&mapItemCounter, mapItems);
					mapData = generate_map(DUNGEON_PRESET_ADVANCED);
					printf("Init advanced dungeon.\n");
					gsi.init = true;
					// init advanced dungeon idk
				}
				if (IsKeyPressed(KEY_R)) {
					set_gamestate(&gsi, GS_INTRO_DUNGEON);
                }
				break;
            }
            default: {
                printf("Gamestate: %i\n", gsi.gameState);
                printf("Error: default case for gamestate?\n");
                break;
            }
        }
        control_entity(&zor, mapData.tiles);
        move_entity(&zor); // perhaps move_entities (once there is ai)
        // move_entity_freely(playerEntity);

        if (IsKeyPressed(KEY_E) && zor.inventoryItemCount > 0) {
            drop_item(zor.inventoryItemCount - 1, &zor, &mapItemCounter, mapItems);
        }

        update_animation_frame(&fantano.animation);
        update_animation_frame(&cyhar.animation);
        // updateAnimationFrame(&zor.animation);
        update_zor_animation(&zor);

        Vector2 camTarget = Vector2Multiply(zor.position, (Vector2) { TILE_SIZE, TILE_SIZE });
        camTarget.x += ((TILE_SIZE - SPRITE_SIZE) / 2) + (SPRITE_SIZE / 2);
        camTarget.y += ((TILE_SIZE - SPRITE_SIZE) / 2) + (SPRITE_SIZE / 4);
        camera.target = camTarget;

        scan_items_for_pickup(&mapItemCounter, mapItems, &zor);

        render_player_inventory(&zor);

        PathList pathList = { .path = NULL, .length = 0 };
        aStarSearch(
            &mapData,
            (Point) {(int)zor.position.x, (int)zor.position.y},
            (Point) {(int)fantano.position.x, (int)fantano.position.y}, 
            & pathList,
            false
        );

        // do rendering
        BeginDrawing();
        {
            ClearBackground(DARKBROWN);
            DrawFPS(10, 5);

            DrawLine(window_width / 2, 0, window_width / 2, window_height, GREEN_SEMI_TRANSPARENT);
            DrawLine(0, window_height / 2, window_width, window_height / 2, GREEN_SEMI_TRANSPARENT);

            BeginMode2D(camera);
            {
                // render map
                for (int row = 0; row < mapData.rows; row++) {
                    for (int col = 0; col < mapData.cols; col++){
                        switch (mapData.tiles[col][row]) {
                            case TILE_WALL: {
                                break;
                            }
                            case TILE_ROOM_ENTRANCE: {
                                DrawRectangle(
                                    col* TILE_SIZE,
                                    row* TILE_SIZE,
                                    TILE_SIZE,
                                    TILE_SIZE,
                                    LIGHTBLUE);
                            }
                            case TILE_CORRIDOR:
                            case TILE_FLOOR: {
                                Color clr = LIGHTGREEN;
                                //if (inList(&path, (Node){col, row})) {
                                /*if (isInPathList(&pathList, (Point) { col, row })) {
								    clr = RED;
                                }*/
                                DrawRectangle(
                                    col * TILE_SIZE,
                                    row * TILE_SIZE, 
                                    TILE_SIZE, 
                                    TILE_SIZE,
                                    clr);

                                if (IsKeyDown(KEY_SPACE)) {
									DrawRectangleLines(
                                        col* TILE_SIZE,
                                        row* TILE_SIZE,
                                        TILE_SIZE,
                                        TILE_SIZE,
                                        BLACK_SEMI_TRANSPARENT);
                                }
                                break;
                            }
                            case TILE_INVALID: {
                                break;
                            }
                            default: {
                                printf("Error (RENDER): Unknown tile type found on map. (%i, %i) = %i\n", col, row, mapData.tiles[col][row]);
                                // printf("? ");
                                break;
                            }
                        }
                    }
                }
      
                // echo map layout to console
                if (IsKeyPressed(KEY_V)) {
                    for (int row = 0; row < mapData.rows; row++) {
                        for (int col = 0; col < mapData.cols; col++) {
                            switch (mapData.tiles[col][row]) {
                            case TILE_WALL: {
                                printf("~ ");
                                break;
                            }
                            case TILE_CORRIDOR:
                            case TILE_FLOOR: {

                                //if (inList(&path, (Node){col, row})) {
                                if (isInPathList(&pathList, (Point) { col, row })) {
                                    printf("P ");
                                } else
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
                                printf("Error (RENDER): Unknown tile type found on map. (%i, %i) = %i\n", col, row, mapData.tiles[col][row]);
                                // printf("? ");
                                break;
                            }
                            }
                        }
                        printf("\n");
                    }
                }

                // render items
				for (int i = 0; i < mapItemCounter; i++) {
                    switch (mapItems[i].type) {
                    case ITEM_NOTHING: {
                        printf("Error: Tried to render ITEM_NOTHING.");
                        break;
                    }
                    case ITEM_SPILLEDCUP: {
                        DrawTextureRec(spilledCupTx, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(mapItems[i].position), WHITE);
                        break;
                    }
                    case ITEM_STICK: {
                        DrawTextureRec(stickTx, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(mapItems[i].position), WHITE);
                        break;
                    }
                    case ITEM_APPLE: {
                        DrawTextureRec(appleTx, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(mapItems[i].position), WHITE);
                        break;
                    }
					default: {
						printf("Idk\n");
						break;
					}
					}
				}

                render_entity(&cyhar);
                render_entity(&fantano);
                render_entity(&zor);
            }
            EndMode2D();
        }
        EndDrawing();
    }

    UnloadTexture(stickTx);
    UnloadTexture(spilledCupTx);
    UnloadTexture(zor.texture);
    UnloadTexture(cyhar.texture);
    UnloadTexture(fantano.texture);

    CloseWindow();
    return 0;
}
