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

#define FADE_DURATION 0.5f

// multiply with deltatime
#define DAMAGE_NOTIF_DURATION 1.0f

#define ATTACK_DELAY_DURATION 0.1f

enum EntityState {
    IDLE,
    MOVE,
    ATTACK_MELEE,
    SKIP_TURN // react to other moves first?
};

enum ItemType {
    ITEM_NOTHING,

    // consumables
    ITEM_APPLE,
    ITEM_SPILLEDCUP,

    // weapons
    ITEM_STICK
};

enum ItemCategory {
    ITEMCAT_NOTHING, // error
    ITEMCAT_WEAPON,
    ITEMCAT_CONSUMABLE
};

typedef struct {
    enum ItemType type;
    enum ItemCategory cat;
    Vector2 position; // grid coordinate position
    // for player to ignore item once dropped
    // enemies should still be able to pickup the item (perhaps some won't want to though)
    //bool prevent_pickup;  // default 0: (0=can pickup) (moved to entity)
    int hp;
} Item;
// ideas:   BasicItem (items with consistent effects)
//          SpecialItem (items that may change? this seems weird..)
//          ItemWear (wear value for each item (such that they are disposable? probably not fun)) 

typedef struct {
    Item items[MAX_INSTANCES];
    int item_counter;
} ItemData;

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
    ENT_NOTHING,
    ENT_ZOR,
    ENT_FANTANO,
    ENT_CYHAR,
    ENT_FLY
};

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

typedef struct {
    enum EntityType ent_type;
    Vector2 position; // grid coordinate position
    Texture2D texture;

    Animation animation;
    enum EntityState state;
    enum Direction direction;
    float lunge_progress;
    int cur_move_anim_extra_frame;

    Item inventory[32];
    int inventory_size;
    int inventory_item_count;
    int equipped_item_index;
    bool can_pickup; // whether they can actaully pickup items in general
    bool prevent_pickup; // temporary state
    bool prevent_drop; // temporary state

    Vector2 original_position;
    bool can_swap_positions;

    int n_turn;
    int max_turns; // max turns per 'turn'
    bool sync_move;

    bool attack_damage_given; // when the attack damage is complete
    int cur_room;
    bool found_target; // for ai
    bool under_attack; // ^^

    int atk; // base damage
    int max_hp; // max hp 
    int hp; // cur hp

    // death animation
    bool faded;
    float fade_timer;

    // attack timing
    float attack_delay;
    bool crit;
} Entity;

#define MOVE_ANIMATION_EXTRA_FRAMES 3

#define MAX_DAMAGE_POPUPS 4

typedef struct {
    float notif_timer;
    int amount;
    Vector2 position;
    bool crit;
} DamagePopup;

typedef struct {
    Entity entities[MAX_INSTANCES];
    int entity_counter;


    // shouldnt be anymore than 2 at once
    DamagePopup damage_popups[MAX_DAMAGE_POPUPS];
} EntityData;
