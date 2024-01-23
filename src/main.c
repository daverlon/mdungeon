/*
    Philosophy:
    Code the game. Nothing more.

    "just type the code for those entities." - Jon Blow
    https://youtu.be/4oky64qN5WI?t=415
*/

#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#define BLACK_SEMI_TRANSPARENT (Color){0, 0, 0, 128}
#define WHITE_SEMI_TRANSPARENT (Color){255, 255, 255, 64}
#define GREEN_SEMI_TRANSPARENT (Color){0, 255, 50, 32}
#define LIGHTBLUE (Color){50, 100, 200, 150}
#define LIGHTGREEN (Color){50, 200, 100, 50}

const int worldWidth = 32*14;
const int worldHeight = 32*8;

#define SPRITE_SIZE 256.0f

#define TILE_SIZE 128

#define GRID_MOVESPEED 3.0f
#define POSITION_THRESHOLD 0.05f

#define LOAD_ZOR_TEXTURE() (LoadTexture("res/zor/zor_spritesheet.png"))
#define LOAD_FANTANO_TEXTURE() (LoadTexture("res/fantano/fantano_idle.png"))
#define LOAD_CYHAR_TEXTURE() (LoadTexture("res/cyhar/cyhar_idle.png"))

#define LOAD_SPILLEDCUP_TEXTURE() (LoadTexture("res/items/spilledcup.png"))

#define MAX_INSTANCES 32

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
} Entity;

typedef struct {
    Vector2 position; // grid coordinate position
} SpilledCup;

Vector2 directionToVector2(enum Direction direction) {
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

void entityController(Entity *en) {

    if (en->isMoving) return;

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

    Vector2 movement = directionToVector2(en->direction);
    if (!Vector2Equals((movement), (Vector2){0}) && shouldMove)  {
        // printf("MOVE!\n");
        // printf("X: %2.0f -> %2.0f\n", en->position.x, en->position.x+1.0f);
        // printf("Y: %2.0f -> %2.0f\n", en->position.y, en->position.y+1.0f);
        en->targetPosition = Vector2Add(en->position, movement);
        en->isMoving = true;
        en->animationState = MOVE;
    }
}

void moveEntity(Entity *en) {
    if (!en->isMoving) return;

    // printf("%2.5f, %2.5f\n", en->position.x, en->position.y);
    // printf("%2.5f, %2.5f\n", en->position.x, en->position.y);

    Vector2 movement = directionToVector2(en->direction);

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

void updateAnimationFrame(Animation *anim) {
    float* ft = &anim->curFrameTime;
    *ft += GetFrameTime();
    int* cf = &anim->curFrame;

    if (*ft >= anim->maxFrameTime) {
        *cf = (*cf % anim->nFrames) + 1;
        *ft = 0.0f;
    }
}

Vector2 PositionToGridPosition(Vector2 pos) {
    return Vector2Subtract(Vector2Multiply(pos, (Vector2){TILE_SIZE, TILE_SIZE}), (Vector2){SPRITE_SIZE/4, SPRITE_SIZE/4});
}

void renderEntity(Entity *en) {

    Vector2 gridPosition = PositionToGridPosition(en->position);
    // printf("%i\n", en->animation.yOffset);
    DrawCircle(gridPosition.x + SPRITE_SIZE/2, gridPosition.y + SPRITE_SIZE/2 + TILE_SIZE/2, 35.0f, BLACK_SEMI_TRANSPARENT);
    DrawTextureRec(en->texture, (Rectangle){en->animation.curFrame * SPRITE_SIZE, en->animation.yOffset + en->direction * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE}, gridPosition, WHITE);
}

Entity createZorEntity(Vector2 pos) {
    Entity en = { 0 };
    // en.animation.maxFrameTime = 0.02f; // run/walk frametime
    en.texture = LOAD_ZOR_TEXTURE();
    en.position = pos;
    return en;
}

void updateZorAnimation(Entity* zor) {

    switch (zor->animationState) {
        case IDLE:
            zor->animation.maxFrameTime = 0.0f;
            zor->animation.yOffset = 0.0f;
            break;
        case MOVE:
            zor->animation.maxFrameTime = 0.02f;
            zor->animation.yOffset = 2048.0f;
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

void CreateSpilledCup(Vector2 pos, int *counter, SpilledCup *cups) {
    cups[*counter] = (SpilledCup){pos};
    (*counter)++;
}

void DeleteSpilledCup(const int index, int *counter, SpilledCup* cups) {
    if (index >= *counter) {
        printf("Warning: Tried to delete spilled cup index: %i, instance counter: %i\n", index, *counter);
        return;
    }

    for (int i = index; i < *counter; i++) {
        cups[index] = cups[index+1];
    }
    (*counter)--;
    printf("Deleted spilled cup index: %i, %i instances left\n", index, *counter);
}

void renderSpilledCups(const int counter, const SpilledCup *cups, Texture2D tx) {
    for (int i = 0; i < counter; i++) {
        DrawTextureRec(tx, (Rectangle){0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE}, PositionToGridPosition(cups[i].position), WHITE);
    }
}

int main(int argc, char* argv[]) {

    // int windowWidth = 1280;
    // int windowHeight = 720;

    // int windowWidth = 1920;
    // int windowHeight = 1080;

    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    InitWindow(windowWidth, windowHeight, "mDungeon");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.offset = (Vector2){windowWidth/2, windowHeight/2};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Entity fantano = {0};
    fantano.texture = LOAD_FANTANO_TEXTURE();
    fantano.position = (Vector2){0.0f, 0.0f};

    Entity cyhar = {0};
    cyhar.texture = LOAD_CYHAR_TEXTURE();
    cyhar.position = (Vector2){3.0f, 3.0f};

    Entity zor = createZorEntity((Vector2){5.0f, 5.0f});

    Entity *playerEntity = {0};
    // playerEntity = &fantano;
    playerEntity = &zor;

    SpilledCup spilledcups[MAX_INSTANCES];
    int spillecups_counter = 0;
    Texture2D spilledcupTexture = LOAD_SPILLEDCUP_TEXTURE();

    CreateSpilledCup((Vector2){2.0f, 2.0f}, &spillecups_counter, spilledcups);
    CreateSpilledCup((Vector2){3.0f, 1.0f}, &spillecups_counter, spilledcups);


    // DeleteSpilledCup(0, &spillecups_counter, spilledcups);
    // DeleteSpilledCup(0, &spillecups_counter, spilledcups);


    for (int i = 0; i < spillecups_counter; i++) {
        printf("%i: %2.5f, %2.5f\n", i, spilledcups[i].position.x, spilledcups[i].position.y);
    }

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
        entityController(playerEntity);
        moveEntity(playerEntity);

        updateAnimationFrame(&fantano.animation);
        updateAnimationFrame(&cyhar.animation);
        // updateAnimationFrame(&zor.animation);
        updateZorAnimation(&zor);

        Vector2 camTarget = Vector2Multiply(playerEntity->position, (Vector2){TILE_SIZE, TILE_SIZE});
        camTarget = Vector2Add(camTarget, (Vector2){SPRITE_SIZE/4, SPRITE_SIZE/4});
        camera.target = camTarget;

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

                renderSpilledCups(spillecups_counter, spilledcups, spilledcupTexture);

                renderEntity(&cyhar);
                renderEntity(&fantano);
                renderEntity(&zor);
            }
            EndMode2D();
        }
        EndDrawing();
    }

    UnloadTexture(spilledcupTexture);
    UnloadTexture(zor.texture);
    UnloadTexture(cyhar.texture);
    UnloadTexture(fantano.texture);
    
    CloseWindow();
    return 0;
}