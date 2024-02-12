#pragma once

#include "raylib.h"
#include "raymath.h"

#define SPRITE_SIZE 256.0f

#define TILE_SIZE 100

// 4.0
#define GRID_MOVESPEED 4.0f
#define POSITION_THRESHOLD 0.05f


#define MAX_INSTANCES 32
#define INVENTORY_SIZE 32

#define DEFAULT_FRAME_COUNT 20

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

    /*int turn_queue[MAX_INSTANCES];
    int queue_size;*/
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
    //bool should_move;
    //Vector2 target_position;
    enum ItemType inventory[32];
    int inventory_item_count;
    bool prevent_pickup;// temporary prevention to pickup dropped items
    bool can_pickup; // whether they can actaully pickup items in general
    Vector2 original_position;
    float lunge_progress;
    bool can_swap_positions;
    int health;
    int n_turn;
    int max_turns; // max turns per 'turn'
    //bool finished_turn;
} Entity;

typedef struct {
    Entity entities[MAX_INSTANCES];
    int entity_counter;
} EntityData;