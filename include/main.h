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
    int window_width;
    int window_height;
    enum GameState game_state;
    bool init;
    int cur_turn_entity_index;
    int cur_turn;
    Vector2 grid_mouse_position;
    char area_name[32];
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
    bool is_swapping;

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

typedef struct {
    Entity* ent;
    int turns_left;
    bool moving;
} EntityTurn;
#define MAX_ENTITY_TURNS 32

#define MOVE_ANIMATION_EXTRA_FRAMES 3

#define MAX_DAMAGE_POPUPS 32

typedef struct {
    float notif_timer;
    int amount;
    Vector2 position;
    bool crit;
} DamagePopup;

typedef struct {
    Entity entities[MAX_INSTANCES];
    int entity_counter;

    bool finished_thinking;

    // shouldnt be anymore than 2 at once
    DamagePopup damage_popups[MAX_DAMAGE_POPUPS];
} EntityData;


enum TileType {
    TILE_INVALID, // error
    TILE_WALL, // default terrain
    TILE_FLOOR,
    TILE_CORRIDOR,
    TILE_ROOM_ENTRANCE
    // TILE_CORRIDOR_MEETING_POINT
};

typedef struct {
    enum TileType type;
    int found;
    int reserved;
} TileData;

// typedef struct {
//     enum TileType type;

// } TileData;

typedef struct {
    // can check if dummy by checking if
    //              cols == 1 && rows == 1
    //              this should be fine
    // bool is_dummy;
    int x;
    int y;
    int cols;
    int rows;
    // bool has_corridor;
    int n_corridors;
    // bool skip; // no room here
} Room;

#define MAX_ROOMS 32

/*
    int n_sectors_x;
    int n_sectors_y;

    int sector_rows;
    int sector_cols;

    int room_width_min;
    int room_height_min;
    int room_width_max;
    int room_height_max;

    //
    // probabilities
    // all GetRandomValue(0,n)==0
    //
    int dummy_chance; 
    int extra_corridor_chance; 
    int corridor_bend_chance;
*/
typedef struct {
    int n_sectors_x;
    int n_sectors_y;

    int sector_cols;
    int sector_rows;

    int room_width_min;
    int room_width_max;

    int room_height_min;
    int room_height_max;

    //
    // probabilities
    // all GetRandomValue(0,n)==0
    //
    // todo: think of a better way to do this?
    // however it works well as is
    // 
    int dummy_chance; 
    int extra_corridor_chance; 
    int corridor_bend_chance;

} MapGenerationConfig;

#define MAX_ROWS 128
#define MAX_COLS 128

typedef struct {
    int n_sectors;

    int cols;
    int rows;
    // enum TileType** tileenum TileType (*tiles)[rows];s;
    // enum TileType (*tiles)[MAX_ROWS];

    //enum TileType tiles[MAX_COLS][MAX_ROWS];
    TileData tiles[MAX_COLS][MAX_ROWS];

    //Vector2 corridor_entrance_points[32];
    // enum TileType (*tiles)[MAX_ROWS];
    Room rooms[MAX_ROOMS];

    float view_distance;

    RenderTexture2D dungeon_texture;
} MapData;

typedef struct {
    GameStateInfo gsi;
    MapData;
    EntityData;
    ItemData;
} FullState;