#include "systems/render.h"
#include "dungeon.h"
#include "main.h"
#include "utils.h"

#include "raylib.h"

#include <stdio.h>

#define WHITE_SEMI_TRANSPARENT (Color){255, 255, 255, 64}
#define GREEN_SEMI_TRANSPARENT (Color){0, 255, 50, 32}
#define LIGHTBLUE (Color){50, 100, 200, 150}
#define LIGHTGREEN (Color){50, 200, 100, 250}
#define LIGHTYELLOW (Color){100, 100, 50, 100}

#define BLACK_SEMI_TRANSPARENT (Color){0, 0, 0, 128}

#define LOAD_SPILLEDCUP_TEXTURE() (LoadTexture("dependencies/res/items/item_spilledcup.png"))
#define LOAD_STICK_TEXTURE() (LoadTexture("dependencies/res/items/item_stick.png"))
#define LOAD_APPLE_TEXTURE() (LoadTexture("dependencies/res/items/item_apple.png"))

#define LOAD_FOREST_GRASS_TILES_TEXTURE() (LoadTexture("dependencies/res/environment/floor_forest_grass.png"))
#define LOAD_FOREST_GRASS_DARK_TILES_TEXTURE() (LoadTexture("dependencies/res/environment/floor_grass_blue.png"))
#define LOAD_FOREST_TERRAIN_TEXTURE() (LoadTexture("dependencies/res/environment/terrain_forest_bush.png"))
#define LOAD_FOREST_DIRT_TILES_TEXTURE() (LoadTexture("dependencies/res/environment/floor_dirt.png"))
#define LOAD_FOREST_ACTIVE_GRASS() (LoadTexture("dependencies/res/environment/floor_active_grass.png"))

#define LOAD_DESERT_TILES_TEXTURE() (LoadTexture("dependencies/res/environment/floor_sand.png"))

#define FOG_AMOUNT 0.3f

static GameStateInfo *gsi;
static Camera2D *camera;
static EntityData *entity_data;
static MapData *map_data;
static ItemData *item_data;

static RenderTexture2D fog_texture;

static Texture2D texture_item_spilledcup;
static Texture2D texture_item_stick;
static Texture2D texture_item_apple;

static Entity *zor;
static Entity *fan;

static Font *fonts;

void render_init(GameStateInfo *_gsi, Camera2D *_camera, EntityData *_entity_data, MapData *_map_data, ItemData *_item_data, Font *fonts) {
    gsi = _gsi;
    camera = _camera;
    entity_data = _entity_data;
    map_data = _map_data;
    item_data = _item_data;

    fog_texture = LoadRenderTexture(gsi->window_width, gsi->window_height);
    texture_item_spilledcup = LOAD_SPILLEDCUP_TEXTURE();
    texture_item_stick = LOAD_STICK_TEXTURE();
    texture_item_apple = LOAD_APPLE_TEXTURE();
}

void render_clean() {
    UnloadRenderTexture(fog_texture);

    // delete item textures
    {
        UnloadTexture(texture_item_spilledcup);
        UnloadTexture(texture_item_stick);
        UnloadTexture(texture_item_apple);
    }
    // delete entity textures
    {
        for (int i = 0; i < entity_data->entity_counter; i++) {
            UnloadTexture(entity_data->entities[i].texture);
        }
    }
}

static void render_entity(Entity* ent, Font* fonts, EntityData* entity_data) {
    Vector2 grid_position = position_to_grid_position(ent->position);
    // Vector2 grid_original_position = position_to_grid_position(ent->original_position);
    // Vector2 grid_infront_position = position_to_grid_position(get_tile_infront_entity(ent));
    // printf("[%i,%i] -> [%i,%i]\n", (int)ent->position.x, (int)ent->position.y, (int)gridPosition.x, (int)gridPosition.y);
    // printf("%i\n", ent->animation.yOffset);

    // offset y
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

    Color tnt = WHITE;
    if (ent->fade_timer > 0.0f) {
        tnt = Fade(WHITE, 1.0f - (ent->fade_timer / FADE_DURATION));
    }

    DrawCircle(
        grid_position.x + (SPRITE_SIZE / 2.0f),
        grid_position.y + (SPRITE_SIZE / 2.0f) + (SPRITE_SIZE / 4.0f),
        35.0f,
        Fade(BLACK, 0.5f - (ent->fade_timer / FADE_DURATION))
    );

    DrawTextureRec(
        ent->texture,
        (Rectangle) {
        ent->animation.cur_frame* SPRITE_SIZE, ent->animation.y_offset + (ent->direction * SPRITE_SIZE), SPRITE_SIZE, SPRITE_SIZE
    },
        grid_position,
            tnt
            );

    //if (ent->fade_timer >= 0.0f) {
    //    // hp bar width
    //    int hp_bar_x = grid_position.x + TILE_SIZE / 1.3 + 2;
    //    int hp_bar_y = grid_position.y + SPRITE_SIZE / 1.2;
    //    int hp_bar_height = 10;
    //    int hp_bar_width = TILE_SIZE;
    //    int hp_cur_width = ((float)ent->hp / (float)ent->max_hp) * (float)hp_bar_width;
    //    DrawRectangle(hp_bar_x, hp_bar_y, hp_bar_width, hp_bar_height, BLACK);
    //    DrawRectangle(hp_bar_x, hp_bar_y, hp_cur_width, hp_bar_height, GREEN);
    //}

    // render damage notification

    {
        // grid position
        // to position
        for (int i = 0; i < MAX_DAMAGE_POPUPS; i++) {
            float timer = entity_data->damage_popups[i].notif_timer;
            int amt = entity_data->damage_popups[i].amount;

            Vector2 start = entity_data->damage_popups[i].position;
            start = position_to_grid_position(start);
            start = (Vector2){ start.x + TILE_SIZE + TILE_SIZE / 4,
                               start.y + TILE_SIZE };
            Vector2 end = (Vector2){ start.x, start.y - 50 };

            if (timer <= 0.0f) continue;

            float progress = (1.0f - (timer / DAMAGE_NOTIF_DURATION));

            //printf("Prog: %2.5f\n", progress);

            Vector2 pos = Vector2Lerp(start, end, progress);

            float font_size = 20.0f;
            Color clr = ORANGE;
            if (entity_data->damage_popups[i].crit) {
                clr = RED;
                font_size += 4.0f;
            }

            DrawTextEx(fonts[0], TextFormat("-%d", amt), (Vector2) { pos.x + 1, pos.y + 1 }, font_size, 1.0f, Fade(BLACK, 1.0f - progress));
            DrawTextEx(fonts[0], TextFormat("-%d", amt), (Vector2) { pos.x, pos.y }, font_size, 1.0f, Fade(clr, 1.0f - progress));
        }
    }

}

void get_item_name(enum ItemType it, char* buf) {
    switch (it) {
    default:
        snprintf(buf, 9, "empty");
        break;
    case ITEM_APPLE:
        snprintf(buf, 6, "apple");
        break;
    case ITEM_STICK:
        snprintf(buf, 6, "stick");
        break;
    case ITEM_SPILLEDCUP:
        snprintf(buf, 12, "spilled cup");
        break;
    }
}

void render_ui(GameStateInfo* gsi, Entity* player, Font* fonts) {

    float scaling = 1280;
    scaling = gsi->window_width / scaling;

    float font_size = 24.0f * scaling;

    {
        char turn_txt[10];
        snprintf(turn_txt, 10, "Turn %i", gsi->cur_turn);

        int xx = gsi->window_width / 2.0f - MeasureTextEx(fonts[0], turn_txt, font_size, 1.0f).x / 2.0f;
        int yy = 30;

        DrawTextEx(fonts[0], turn_txt, (Vector2) { xx + 2, yy + 2 }, font_size, 1.0f, Fade(BLACK, 0.7f));
        DrawTextEx(fonts[0], turn_txt, (Vector2) { xx, yy }, font_size, 1.0f, WHITE);
    }

    int y = 30;

    //y += 25;

    // hp bar
    {
        int w = 300 * scaling;
        /*int h = (float)window_height * 0.0166666667;*/
        int h = 12 * scaling;
        //int x = window_width / 2 - w / 2;
        int x = gsi->window_width - w - 30;

        Rectangle hp_bar = (Rectangle){ x, y, w, h };
        int hp_hp = ((float)player->hp / (float)player->max_hp) * hp_bar.width;

        //DrawRectangleGradientV(x, y, w, h, RED, (Color){64, 0, 0, 255});
        DrawRectangleGradientV(x+1, y, w-2, h, (Color) { 64, 0, 0, 255 }, RED);
        DrawRectangleGradientV(x+1, y, hp_hp-2, h, GREEN, DARKGREEN);
        DrawRectangleRoundedLines(hp_bar, 0.5f, 1.0f, 2, BLACK);

        char txt[10];
        snprintf(txt, 10, "%i/%i", player->hp, player->max_hp);

        int yy = y - 1;
        int xx = x + 2;
        DrawTextEx(fonts[0], txt, (Vector2) { xx + 2, yy + 2 }, font_size, 1.0f, Fade(BLACK, 0.7f));
        DrawTextEx(fonts[0], txt, (Vector2) { xx, yy }, font_size ,1.0f, WHITE);
    }

    y += 25 * scaling;

    // item hp bar
    {
        int w = 210 * scaling;
        int h = 12 * scaling;
        int x = gsi->window_width - w - 30;

        int item_i = player->equipped_item_index;
        Item* equipped_item = &player->inventory[item_i];

        Rectangle hp_bar = (Rectangle){ x, y, w, h };
        int hp_hp = ((float)equipped_item->hp / (float)player->max_hp) * hp_bar.width;

        //DrawRectangleGradientV(x, y, w, h, GRAY, (Color){30, 30, 30, 255});
        DrawRectangleGradientV(x, y, w, h, (Color) { 30, 30, 30, 255 }, GRAY);
        //DrawRectangleGradientV(x, y, hp_hp, h, BLUE, (Color) { 0, 0, 202, 255 });
        DrawRectangleGradientV(x, y, hp_hp, h, ORANGE, (Color) { 255 - 90, 161 - 90, 0, 255 });
        DrawRectangleRoundedLines(hp_bar, 0.5f, 1.0f, 2, BLACK);

        char txt[16];
        get_item_name(player->inventory[item_i].type, txt);

        // Vector2 fs = MeasureTextEx(fonts[0], txt, font_size, 1.0f);
        //x = window_width - 30 - fs.x - 3;

        DrawTextEx(fonts[0], txt, (Vector2) { x + 4, y + 2 }, font_size, 1.0f, Fade(BLACK, 0.7f));
        DrawTextEx(fonts[0], txt, (Vector2) { x + 2, y }, font_size, 1.0f, WHITE);

    }

    // area name
    {
        int xx = 30;
        int yy = 30;

        DrawTextEx(fonts[0], gsi->area_name, (Vector2) { xx + 4, yy + 2 }, font_size, 1.0f, Fade(BLACK, 0.7f));
        DrawTextEx(fonts[0], gsi->area_name, (Vector2) { xx + 2, yy }, font_size, 1.0f, WHITE);
   }

    // inventory
    {
        const int gap = 30 * scaling;
        const int c = player->inventory_item_count;
        // int size = 30 * scaling;
        int xx = 30;

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
                DrawTextEx(fonts[0], "Spilled Cup", (Vector2) { xx, y + (i * gap) }, font_size, 1.0f, WHITE);
                break;
            }
            case ITEM_STICK: {
                DrawTextEx(fonts[0], "Stick", (Vector2) { xx, y + (i * gap) }, font_size, 1.0f, WHITE);
                break;
            }
            case ITEM_APPLE: {
                DrawTextEx(fonts[0], "Apple", (Vector2) { xx, y + (i * gap) }, font_size, 1.0f, WHITE);
                break;
            }
            }
        }

    }
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

void render_update() {

    int prev_width = gsi->window_width;
    if (IsWindowResized()) {
        gsi->window_width = GetScreenWidth();
        gsi->window_height = GetScreenHeight();
        camera->offset = (Vector2){ gsi->window_width / 2.0f, gsi->window_height / 2.0f };

        UnloadRenderTexture(fog_texture);
        fog_texture = LoadRenderTexture(gsi->window_width, gsi->window_height);

        camera->zoom *= ((float)gsi->window_width / (float)prev_width);
    }

    // do rendering
    BeginDrawing();
    {
        ClearBackground(BLACK);

        DrawLine(gsi->window_width / 2, 0, gsi->window_width / 2, gsi->window_height, GREEN_SEMI_TRANSPARENT);
        DrawLine(0, gsi->window_height / 2, gsi->window_width, gsi->window_height / 2, GREEN_SEMI_TRANSPARENT);

        BeginMode2D(*camera);
        {
            // render map
            DrawTexture(map_data->dungeon_texture.texture, 0, 0, WHITE);
            for (int row = 0; row < map_data->rows; row++) {
                for (int col = 0; col < map_data->cols; col++) {
                    if (IsKeyDown(KEY_SPACE) && map_data->tiles[col][row].type != TILE_WALL) {
                        Color clr = BLACK_SEMI_TRANSPARENT;

                        gsi->grid_mouse_position = GetMousePosition();
                        gsi->grid_mouse_position = GetScreenToWorld2D(gsi->grid_mouse_position, *camera);
                        gsi->grid_mouse_position = Vector2Divide(gsi->grid_mouse_position, (Vector2) { TILE_SIZE, TILE_SIZE });
                        gsi->grid_mouse_position.x = floorf(gsi->grid_mouse_position.x);
                        gsi->grid_mouse_position.y = floorf(gsi->grid_mouse_position.y);
                        //print_vector2(grid_mouse_position);

                        if (gsi->grid_mouse_position.x == col && gsi->grid_mouse_position.y == row) {
                            clr = YELLOW;
                        }

                        if (Vector2Equals(get_tile_infront_entity(zor), (Vector2) { col, row })) {
                            clr = ORANGE;
                        }

                        Vector2 mouse_grid_pos_dir = Vector2Clamp(
                            Vector2Subtract(gsi->grid_mouse_position, zor->original_position),
                            (Vector2) {
                            -1.0f, -1.0f
                        },
                            (Vector2) {
                            1.0f, 1.0f
                        });

                        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
                            print_vector2(gsi->grid_mouse_position);

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
            for (int i = 0; i < &item_data->item_counter; i++) {
                if (!is_item_visible(zor, &item_data->items[i], &map_data)) continue;

                switch (item_data->items[i].type) {
                case ITEM_NOTHING: {
                    printf("Error: Tried to render ITEM_NOTHING.");
                    break;
                }
                case ITEM_SPILLEDCUP: {
                    DrawTextureRec(texture_item_spilledcup, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(item_data->items[i].position), WHITE);
                    break;
                }
                case ITEM_STICK: {
                    DrawTextureRec(texture_item_stick, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(item_data->items[i].position), WHITE);
                    break;
                }
                case ITEM_APPLE: {
                    DrawTextureRec(texture_item_apple, (Rectangle) { 0.0f, 0.0f, SPRITE_SIZE, SPRITE_SIZE }, position_to_grid_position(item_data->items[i].position), WHITE);
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
        //if (false)
        {
            BeginTextureMode(fog_texture);
            BeginBlendMode(BLEND_SUBTRACT_COLORS);
            ClearBackground(Fade(BLACK, FOG_AMOUNT)); // Dim the whole screen

            if (zor->cur_room != -1) {
                Room* rm = &map_data->rooms[zor->cur_room];

                // Calculate the room's position and size based on TILE_SIZE
                Vector2 tl = (Vector2){ rm->x * TILE_SIZE, rm->y * TILE_SIZE };
                tl.x -= TILE_SIZE / 4;
                tl.y -= TILE_SIZE / 4;
                tl = GetWorldToScreen2D(tl, *camera);

                Vector2 br = (Vector2){ (rm->x + rm->cols) * TILE_SIZE, (rm->y + rm->rows) * TILE_SIZE };
                br = GetWorldToScreen2D(br, *camera);
                br.x += TILE_SIZE / 4;
                br.y += TILE_SIZE / 4;

                //Vector2 delta = Vector2Subtract(br, tl);
                Vector2 delta = (Vector2){
                    br.x - tl.x,
                    br.y - tl.y
                };
                tl.y += delta.y;

                //DrawRectangle(tl.x, window_height - tl.y, delta.x, delta.y, Fade(BLACK, 0.5f));
                Rectangle light = (Rectangle){ tl.x, gsi->window_height - tl.y, delta.x, delta.y };
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
                Vector2 screen_pos = GetWorldToScreen2D(pos, *camera);

                float circle_radius = TILE_SIZE * pow(map_data->view_distance, 0.5f) * camera->zoom;

                DrawCircle(screen_pos.x, screen_pos.y, circle_radius, Fade(BLACK, FOG_AMOUNT));
            }

            EndBlendMode();
            EndTextureMode(); // End drawing to texture

            // Draw the fog texture over the whole screen
            DrawTexture(fog_texture.texture, 0, 0, WHITE);
        }

        BeginMode2D(*camera);
        // y sort render entities
        {
            bool rendered[MAX_INSTANCES] = { false };
            for (int i = 0; i < entity_data->entity_counter; i++) {
                int lowest_y = 99999;
                int index_to_render = -1;

                for (int e = 0; e < entity_data->entity_counter; e++) {
                    Entity* ent = &entity_data->entities[e];

                    if (e != 0) {
                        // if (!is_entity_visible(zor, ent, &map_data, false)) continue;
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
                    render_entity(&entity_data->entities[index_to_render], fonts, &entity_data);
                    rendered[index_to_render] = true;
                }
            }
        }
        EndMode2D();


        DrawFPS(10, 5);
        render_ui(&gsi, zor, fonts);

    } // end rendering
    EndDrawing();
}
