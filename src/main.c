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

#define BLACK_SEMI_TRANSPARENT (Color){0, 0, 0, 128}
#define WHITE_SEMI_TRANSPARENT (Color){255, 255, 255, 64}
#define GREEN_SEMI_TRANSPARENT (Color){0, 255, 50, 32}
#define LIGHTBLUE (Color){50, 100, 200, 150}
#define LIGHTGREEN (Color){50, 200, 100, 50}

const int worldWidth = 32*14;
const int worldHeight = 32*8;

#define SPRITE_SIZE 128.0f

#define TILE_SIZE 128

#define GRID_MOVESPEED 3.0f
#define POSITION_THRESHOLD 0.05f

#define LOAD_ZOR_TEXTURE() (LoadTexture("res/zor/zor_spritesheet.png"))
#define LOAD_FANTANO_TEXTURE() (LoadTexture("res/fantano/fantano_idle.png"))
#define LOAD_CYHAR_TEXTURE() (LoadTexture("res/cyhar/cyhar_idle.png"))

#define LOAD_SPILLEDCUP_TEXTURE() (LoadTexture("res/items/spilledcup.png"))

#define MAX_INSTANCES 32
#define INVENTORY_SIZE 32

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
    MOVE
};

enum ItemType {
    ITEM_NOTHING,
    ITEM_SPILLEDCUP
};

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
            return (Vector2){0.0f, 1.0f};
        case DOWNRIGHT:
            return (Vector2){1.0f, 1.0f};
        case RIGHT:
            return (Vector2){1.0f, 0.0f};
        case UPRIGHT:
            return (Vector2){1.0f, -1.0f};
        case UP:
            return (Vector2){0.0f, -1.0f};
        case UPLEFT:
            return (Vector2){-1.0f, -1.0f};
        case LEFT:
            return (Vector2){-1.0f, 0.0f};
        case DOWNLEFT:
            return (Vector2){-1.0f, 1.0f};
        default:
            // Handle invalid direction
            return (Vector2){0.0f, 0.0f};
    }
}

void control_entity(Entity *en) {

    if (en->isMoving) return;
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

    // this may be a bit scuffed. unecessary extra checks here?
    Vector2 movement = direction_to_vector2(en->direction);
    if (!Vector2Equals((movement), (Vector2){0}) && shouldMove)  {
        // printf("MOVE!\n");
        // printf("X: %2.0f -> %2.0f\n", en->position.x, en->position.x+1.0f);
        // printf("Y: %2.0f -> %2.0f\n", en->position.y, en->position.y+1.0f);
        en->targetPosition = Vector2Add(en->position, movement);
        en->isMoving = true;
        en->animationState = MOVE;
    }
}

void move_entity(Entity *en) {
    // move entity grid position
    // (when entity is moving)
    if (!en->isMoving) return;

    // printf("%2.5f, %2.5f\n", en->position.x, en->position.y);
    // printf("%2.5f, %2.5f\n", en->position.x, en->position.y);

    const Vector2 movement = direction_to_vector2(en->direction);

    // linear interpolation
    // en->position.x = Lerp(en->position.x, en->targetPosition.x, t);
    // en->position.y = Lerp(en->position.y, en->targetPosition.y, t);

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

void update_animation_frame(Animation *anim) {
    float* ft = &anim->curFrameTime;
    *ft += GetFrameTime();
    int* cf = &anim->curFrame;

    if (*ft >= anim->maxFrameTime) {
        *cf = (*cf % anim->nFrames) + 1;
        *ft = 0.0f;
    }
}

Vector2 position_to_grid_position(Vector2 pos) {
    return Vector2Subtract(Vector2Multiply(pos, (Vector2){TILE_SIZE, TILE_SIZE}), (Vector2){SPRITE_SIZE/2, SPRITE_SIZE/2});
}

void render_entity(Entity *en) {
    Vector2 gridPosition = position_to_grid_position(en->position);
    // printf("%i\n", en->animation.yOffset);
    DrawCircle(gridPosition.x + SPRITE_SIZE/2, gridPosition.y + SPRITE_SIZE/2 + TILE_SIZE/2, 35.0f, BLACK_SEMI_TRANSPARENT);
    DrawTextureRec(
        en->texture, 
        (Rectangle){en->animation.curFrame * SPRITE_SIZE, en->animation.yOffset + en->direction * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE}, 
        gridPosition, 
        WHITE);
}

Entity create_zor_entity_instance(Vector2 pos) {
    Entity en = { 0 };
    // en.animation.maxFrameTime = 0.02f; // run/walk frametime
    // en.texture = LOAD_ZOR_TEXTURE();
    en.position = pos;
    return en;
}

void update_zor_animation(Entity* zor) {

    // higher fps seems to speed this up
    switch (zor->animationState) {
        case IDLE:
            zor->animation.maxFrameTime = 0.0f;
            zor->animation.yOffset = 0.0f;
            break;
        case MOVE:
            zor->animation.maxFrameTime = 0.02f;
            // zor->animation.yOffset = 2048.0f;
            zor->animation.yOffset = SPRITE_SIZE * 8.0f;
            break;
        default:
            zor->animation.maxFrameTime = 0.0f;
            zor->animation.yOffset = 0.0f;
            break;
    }

    float* ft = &zor->animation.curFrameTime;
    *ft += GetFrameTime();
    int* cf = &zor->animation.curFrame;

    if (*ft >= zor->animation.maxFrameTime) {
        *cf = (*cf % zor->animation.nFrames) + 1;
        *ft = 0.0f;
    }
}

void create_item_instance(Item item, int *counter, Item *items) {
    if (item.type == ITEM_NOTHING) {
        printf("Error: Tried to create ITEM_NOTHING.\n");
        return;
    }
    items[*counter] = item;
    (*counter)++;
}

void delete_item(const int index, int *counter, Item *items) {
    if (index >= *counter) {
        printf("Warning: Tried to delete item index: %i, instance counter: %i\n", index, *counter);
        return;
    }

    for (int i = index; i < *counter; i++) {
        items[i] = items[i+1];
    }
    (*counter)--;
    printf("Deleted item index: %i, %i instances left on map.\n", index, *counter);
}

// all items should probably not be on 1 texture.
// only certain items can appear at a time
// todo: un-generic this function
void render_items_on_map(int *counter, Item *items, Texture2D tx) {
    for (int i = 0; i < *counter; i++) {
        switch (items[i].type) {
            default:
                break;
            case ITEM_NOTHING:
                // it should not be possible to iterate over ITEM_NOTHING.
                printf("Error: Tried to render ITEM_NOTHING.");
                break;
            case ITEM_SPILLEDCUP:
                DrawTextureRec(tx, (Rectangle){0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE}, position_to_grid_position(items[i].position), WHITE);
                break;
        }
    }
}

void pickup_item(const int index, Item *mapItems, Entity *entity) {
    // add item to entity inventory
    // this should be called with delete_item called after
    if (entity->inventoryItemCount < INVENTORY_SIZE) {
        entity->inventory[entity->inventoryItemCount] = mapItems[index].type;
        entity->inventoryItemCount++;
    }
}

void scan_items_for_pickup(int *counter, Item *mapItems, Entity *entity) {
    for (int i = 0; i < *counter; i++) {
        Item *item = &mapItems[i];
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
        entity->inventory[i] = entity->inventory[i+1];
    }
    entity->inventoryItemCount--;
    printf("Deleted inventory item index: %i, %i items remaining.\n", index, entity->inventoryItemCount);
}

void drop_item(const int index, Entity* entity, int *mapItemCounter, Item *mapItems) {
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
    mapItems[*mapItemCounter] = (Item){entity->inventory[index], entity->position, true};
    (*mapItemCounter)++;
    delete_item_from_entity_inventory(index, entity);
}

void render_player_inventory(Entity *player) {
    // for debug/development purposes
    const int c = player->inventoryItemCount;
    const int yOffset = 100;
    const int gap = 30;
    DrawText("Inventory", 10, yOffset - gap, 24, WHITE);
    for (int i = 0; i < c; i++) {
        switch (player->inventory[i]) {
            default:
                printf("idk\n");
                break;
            case ITEM_NOTHING:
                printf("Error: ITEM_NOTHING present in player inventory.");
                break;
            case ITEM_SPILLEDCUP:
            {
                char msg[20];
                sprintf(msg, "%i: ITEM_SPILLEDCUP", i+1);
                DrawText(msg, 10, yOffset + (i * gap), 24, WHITE);
                // printf("Item %i: ITEM_SPILLEDCUP\n", i);
                break;
            }
        }
    }
}

int main(void/*int argc, char* argv[]*/) {

    int windowWidth = 1920;
    int windowHeight = 1080;

    // int windowWidth = GetScreenWidth();
    // int windowHeight = GetScreenHeight();
    InitWindow(windowWidth, windowHeight, "mDungeon");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.offset = (Vector2){windowWidth/2, windowHeight/2};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Entity fantano = {0};
    // fantano.texture = LOAD_FANTANO_TEXTURE();
    fantano.position = (Vector2){0.0f, 0.0f};

    Entity cyhar = {0};
    // cyhar.texture = LOAD_CYHAR_TEXTURE();
    cyhar.position = (Vector2){3.0f, 3.0f};

    Entity zor = create_zor_entity_instance((Vector2){5.0f, 5.0f});
    zor.texture = LOAD_ZOR_TEXTURE();
    // SetTextureFilter(zor.texture, TEXTURE_FILTER_POINT);
    // GenTextureMipmaps(zor.texture, 0);  // Disable mipmaps


    Entity *playerEntity = {0};
    playerEntity = &zor;
    // playerEntity = &fantano;

    Item mapItems[MAX_INSTANCES];
    int mapItemCounter = 0;
    // Texture2D itemTexture = LOAD_SPILLEDCUP_TEXTURE();

    create_item_instance((Item){ITEM_SPILLEDCUP, (Vector2){2.0f, 2.0f}, false}, &mapItemCounter, mapItems);
    create_item_instance((Item){ITEM_SPILLEDCUP, (Vector2){3.0f, 1.0f}, false}, &mapItemCounter, mapItems);
    // delete_item(0, &mapItemCounter, mapItems);

    while (!WindowShouldClose()) { 

        if (IsWindowResized()) {
            windowWidth = GetScreenWidth();
            windowHeight = GetScreenHeight();
            camera.offset = (Vector2){windowWidth/2, windowHeight/2};
        }

        // handle events
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            camera.offset = Vector2Add(camera.offset, GetMouseDelta());
        }
        int scroll = GetMouseWheelMove();
        if (scroll != 0.0f) { 
            camera.zoom += scroll * 0.1f;
            printf("Scroll: %2.0f\n", camera.zoom);
        }

        // update game logic
        control_entity(playerEntity);
        move_entity(playerEntity); // perhaps move_entities (once there is ai)
        // move_entity_freely(playerEntity);

        if (IsKeyPressed(KEY_E) && playerEntity->inventoryItemCount > 0)  {
            drop_item(playerEntity->inventoryItemCount-1, playerEntity, &mapItemCounter, mapItems);
        }

        update_animation_frame(&fantano.animation);
        update_animation_frame(&cyhar.animation);
        // updateAnimationFrame(&zor.animation);
        update_zor_animation(&zor);

        Vector2 camTarget = Vector2Multiply(playerEntity->position, (Vector2){TILE_SIZE, TILE_SIZE});
        camTarget = Vector2Add(camTarget, (Vector2){SPRITE_SIZE/4, SPRITE_SIZE/4});
        camera.target = camTarget;

        scan_items_for_pickup(&mapItemCounter, mapItems, playerEntity);

        render_player_inventory(playerEntity);

        // do rendering
        BeginDrawing();
        {
            ClearBackground(DARKBROWN);
            DrawFPS(10, 5);

            DrawLine(windowWidth/2, 0, windowWidth/2, windowHeight, GREEN_SEMI_TRANSPARENT);
            DrawLine(0, windowHeight/2, windowWidth, windowHeight/2, GREEN_SEMI_TRANSPARENT);

            BeginMode2D(camera); 
            {
                DrawRectangle(0, SPRITE_SIZE/4, 8 * TILE_SIZE, 8 * TILE_SIZE, LIGHTGREEN);
                for (int x = 0; x < 8; x++) {
                    for (int y = 0; y < 8; y++) {
                        DrawRectangleLines(x * TILE_SIZE, SPRITE_SIZE/4 + y * TILE_SIZE, TILE_SIZE, TILE_SIZE, WHITE_SEMI_TRANSPARENT);
                    }
                }

                DrawRectangleLines(0, SPRITE_SIZE/4, 8 * TILE_SIZE, 8 * TILE_SIZE, BLACK);

                // render_spilledcups(spillecups_counter, spilledcups, spilledcupTexture);
                // render_items_on_map(&mapItemCounter, mapItems, itemTexture);

                // render_entity(&cyhar);
                // render_entity(&fantano);
                render_entity(&zor);
            }
            EndMode2D();
        }
        EndDrawing();
    }

    // UnloadTexture(itemTexture);
    UnloadTexture(zor.texture);
    UnloadTexture(cyhar.texture);
    UnloadTexture(fantano.texture);
    
    CloseWindow();
    return 0;
}
