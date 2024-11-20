#include "systems/entity_think.h"

#include "stdio.h"
#include "utils.h"


static void reset_tile_reservations(MapData* map_data) {
  for (int col = 0; col < MAX_COLS; col++) {
    for (int row = 0; row < MAX_ROWS; row++) {
      map_data->tiles[col][row].reserved = false;
    }
  }
}

static bool sync_moving_entity_exists(EntityData* entity_data) {
    for (int i = 0; i < entity_data->entity_counter; i++) {
        if (entity_data->entities[i].sync_move)
            return true;
    }
    return false;
}

static bool is_entity_dead(Entity* ent) {
    return (ent->hp <= 0);
}

static bool entity_finished_turn(Entity* ent) {
    if (is_entity_dead(ent)) {
        return ent->faded;
    }
    else {
        return (ent->state == IDLE && ent->n_turn >= ent->max_turns);
    }
}


static int get_melee_attack_damage(Entity* ent, int* self_damage, bool* crit) {
    float damage = (float)ent->atk;

    // todo: ui etc for equipped item
    if (ent->inventory_item_count <= 0) return (int)roundf(damage);

    switch (ent->inventory[ent->equipped_item_index].type) {
    case ITEM_NOTHING: {
        if (GetRandomValue(0, 9)) {
            damage *= 1.7f;
            *crit = true;
        }
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
        if (GetRandomValue(0, 2) == 0) {
            damage *= 1.7f;
            *crit = true;
        }

        *self_damage = GetRandomValue(45, 50);
        break;
    }
    default:
        break;
    }

    damage = value_variation(damage, 5);
    //printf("Damage: %2.5f\n", fdamage);

    return damage;
}

static void process_attack(Entity* ent, EntityData* entity_data) {
    enum EntityState attack_state = ent->state;

    switch (attack_state) {
    case ATTACK_MELEE: {

        Vector2 tile = get_tile_infront_entity(ent);

        int id = 0;
        if (any_entity_exists_on_tile(tile.x, tile.y, entity_data, NULL, &id)) {
            if (!ent->attack_damage_given) {
                if (ent->animation.cur_frame > 7) {
                    int self_damage = 0;

                    bool crit = false;

                    int damage = get_melee_attack_damage(ent, &self_damage, &crit);
                    //printf("Damage: %i ||| Self Damage: %i\n", damage, self_damage);

                    apply_damage(&entity_data->entities[id], ent, damage, true, crit, entity_data);

                    ent->inventory[ent->equipped_item_index].hp -= self_damage;
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

static void process_entity_state(Entity* ent, EntityData* entity_data) {
    switch (ent->state) {
    case IDLE: {
        // not really supposed to trigger here
        //reset_entity_state(ent, false);
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
        if (ent->attack_delay < ATTACK_DELAY_DURATION) {
            if (ent->animation.cur_frame != 0)
                ent->animation.cur_frame = 0;
            ent->attack_delay += GetFrameTime();
            break;
        }
        process_attack(ent, entity_data);
        switch (ent->ent_type) {
        default:
            lunge_entity(ent, 1.0f, 2.5f);
            break;
        case ENT_ZOR:
            lunge_entity(ent, 1.2f, 2.9f);
            break;
        case ENT_FLY:
            lunge_entity(ent, 1.4f, 3.0f);
            break;
        }
        break;
    }
    default:
        break;
    }
}




void process_entity_turn_queue(GameStateInfo* gsi, EntityData* entity_data, ItemData* item_data, MapData* map_data) {

    Entity* zor = &entity_data->entities[0];

    // process_initial_entity_think(gsi, entity_data, item_data, map_data);

    // if (entity_data->entity_counter > 0)
    // if (entity_finished_turn(&entity_data->entities[entity_data->entity_counter-1] && !sync_moving_entity_exists(entity_data))) {
        // entity_data->finished_thinking = false;
        // return;
    // }

    /*for (int i = 0; i < entity_data->entity_counter; i++) {
        EntityTurn* ent_turn = &entity_turn_data[i];
        if (ent_turn->ent == NULL) continue;
        if (ent_turn->turns_left > 0) {
            // found incomplete entity turn
            if ()
        }
    } */
    // prioritise swapping entity first

    if (sync_moving_entity_exists(entity_data)) {

        for (int i = 0; i < entity_data->entity_counter; i++) {
            Entity* ent = &entity_data->entities[i];

            if (!ent->sync_move && !ent->is_swapping)
                continue;

            //if (ent->is_swapping && !entity_finished_turn(ent)) {
            //  process_entity_state(ent, entity_data);
            //  continue;
            //}

            if (is_entity_dead(ent)) {
                ent->state = IDLE;
                ent->sync_move = false;
                ent->is_swapping = false;
            }

            if (entity_finished_turn(ent)) {
                // when an in sync entity has finished their turn
                gsi->cur_turn_entity_index++;
                ent->n_turn = 0;
                if (gsi->cur_turn_entity_index >= entity_data->entity_counter) {
                    gsi->cur_turn_entity_index = 0;
                    gsi->cur_turn++;
                }
                ent->sync_move = false;

                ent->prevent_pickup = false;
                ent->prevent_drop = false;
            }
            else {
                // if not, rethink turn and process it
                if (zor->is_swapping && !ent->is_swapping) {
                    entity_think(ent, zor, map_data, entity_data, item_data, gsi->grid_mouse_position);
                }

                if (ent->state == IDLE || ent->state == SKIP_TURN) {
                    entity_think(ent, zor, map_data, entity_data, item_data, gsi->grid_mouse_position);
                }

                process_entity_state(ent, entity_data);
            }
        }
    }
    else {
        // no sync moving entities exist currently

        reset_tile_reservations(map_data);

        Entity* this_ent = &entity_data->entities[gsi->cur_turn_entity_index];

        if (this_ent->state != MOVE)
            entity_think(this_ent, zor, map_data, entity_data, item_data, gsi->grid_mouse_position);

        // check if sync entities should exist for this turn
        // and log them
        if (this_ent->state == MOVE) {
            for (int i = gsi->cur_turn_entity_index + 1; i < entity_data->entity_counter; i++) {
                Entity* ent = &entity_data->entities[i];

                entity_think(ent, zor, map_data, entity_data, item_data, gsi->grid_mouse_position);
                if (ent->state != MOVE && ent->state != SKIP_TURN) {
                    reset_entity_state(ent, false);
                    break;
                }
                this_ent->sync_move = true;
                ent->sync_move = true;
            }
        }

        // if there are none, process individual entity as normal
        if (!sync_moving_entity_exists(entity_data)) {

            Entity* this_ent = &entity_data->entities[gsi->cur_turn_entity_index];

            if (this_ent->state != MOVE)
                entity_think(this_ent, zor, map_data, entity_data, item_data, gsi->grid_mouse_position);

            if (is_entity_dead(this_ent))
                this_ent->state = IDLE;

            process_entity_state(this_ent, entity_data);

            // entity finished turn?
            if (entity_finished_turn(this_ent)) {
                gsi->cur_turn_entity_index++;
                this_ent->n_turn = 0;
                if (gsi->cur_turn_entity_index >= entity_data->entity_counter) {
                    gsi->cur_turn_entity_index = 0;
                    gsi->cur_turn++;
                }
                this_ent->prevent_pickup = false;
                this_ent->prevent_drop = false;
            }
        }
    }
}