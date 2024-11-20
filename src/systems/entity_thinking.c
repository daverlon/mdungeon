#include "systems/entity_thinking.h"

#include "pathfinding.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void set_entity_position(Entity* ent, Vector2 pos) {
    ent->position = pos;
    ent->original_position = pos;
    //map_data->tiles[(int)pos.x][(int)pos.y].reserved = true;
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


bool any_entity_exists_on_tile(int col, int row, const EntityData* entity_data, Entity* ignore, int* out) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < entity_data->entity_counter; i++) {
        const Entity* ent = &entity_data->entities[i];

        if (ignore != NULL && ent == ignore) continue;

        Vector2 ent_target_position = get_tile_infront_entity(ent);
        if ((ent->state == MOVE && Vector2Equals(ent_target_position, pos))
            || (ent->state != MOVE && Vector2Equals(ent->original_position, pos))
            || (ent->is_swapping && Vector2Equals(ent->original_position, pos)) ) {
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




bool entity_reached_destination(Entity* ent) {
    return Vector2Equals(ent->position, ent->original_position);
}

bool item_exists_on_tile(int col, int row, const ItemData* item_data, int* index) {
    Vector2 pos = (Vector2){ col, row };
    for (int i = 0; i < item_data->item_counter; i++) {
        if (Vector2Equals(item_data->items[i].position, pos)) {
            if (index != NULL) *index = i;
            return true;
        }
    }
    return false;
}

void reset_entity_state(Entity* ent, bool use_turn) {
    ent->state = IDLE;
    ent->attack_delay = 0.0f;
    //ent->original_position = ent->position;
    if (use_turn) {
        ent->n_turn++;
        ent->attack_damage_given = false;
    }
    ent->is_swapping = false;
}

void move_entity_forward(Entity* ent/*,MapData* map_data*/) {
    //if (ent->state != MOVE) return;

    const Vector2 movement = direction_to_vector2(ent->direction);
    //const Vector2 normalized_movement = Vector2Normalize(movement); // Normalize the movement vector
    const Vector2 targ = Vector2Add(ent->original_position, movement);

    /*if (map_data->tiles[(int)targ.x][(int)targ.y].reserved) {
        
    }*/

    //map_data->tiles[(int)targ.x][(int)targ.y].reserved = true;
    //map_data->tiles[(int)ent->original_position.x][(int)ent->original_position.y].reserved = false;

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




void control_entity(Entity* ent, MapData* map_data, EntityData* entity_data, ItemData* item_data, Vector2 grid_mouse_position) {

    // if no key is held, ensure that isMoving is set to false
    if (ent->state != IDLE) return;

    if (IsKeyPressed(KEY_F)) {
        // check ground
        int buf = -1;
        if (item_exists_on_tile(ent->original_position.x, ent->original_position.y, item_data, &buf)) {
            // pickup
            if (!ent->prevent_pickup) {
                pickup_item(buf, item_data, ent);
                ent->prevent_drop = true;
                reset_entity_state(ent, true);
                return;
            }
        }
        else {
            if (!ent->prevent_drop) {
                if (ent->inventory_item_count > 0) {
                    drop_item(ent->inventory_item_count - 1, ent, item_data);
                    ent->prevent_pickup = true;
                    reset_entity_state(ent, true);
                    return;
                }
            }
        }
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
        //ent->animation.cur_frame = 0;
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
            if (swapi != -1) {
                if (!entity_data->entities[swapi].can_swap_positions) {
                    reset_entity_state(ent, false);
                    return;
                } 
                else {
                    ent->is_swapping = true;
                    entity_data->entities[swapi].sync_move = true;
                    entity_data->entities[swapi].is_swapping = true;
                    entity_data->entities[swapi].direction = vector_to_direction(Vector2Subtract(ent->position, entity_data->entities[swapi].position));
                    ent->state = MOVE;
                    return;
                }
            }
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


void apply_damage(Entity* to, Entity* from, int amount, bool change_direction, bool crit, EntityData* entity_data) {
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

    if (change_direction && to->state == IDLE && !Vector2Equals(direction, (Vector2){0.0f, 0.0f}))
        to->direction = vector_to_direction(direction);

    from->attack_damage_given = true;
    to->found_target = true; // for ai

    // shouldnt need more than 2 holders for damage popup
    for (int i = 0; i < MAX_DAMAGE_POPUPS; i++) {
        // active timer
        if (entity_data->damage_popups[i].notif_timer > 0.0f)
            continue;

        entity_data->damage_popups[i].notif_timer = DAMAGE_NOTIF_DURATION;
        entity_data->damage_popups[i].amount = amount;
        entity_data->damage_popups[i].position = to->original_position;
        entity_data->damage_popups[i].crit = crit;
        break;
    }
}


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

    int index = GetRandomValue(0, entity->inventory_item_count - 1);
    item_data->items[item_data->item_counter] = entity->inventory[index];
    item_data->item_counter++;
    delete_item_from_entity_inventory(index, entity);
    //entity->prevent_pickup = true;
}

void lunge_entity(Entity* ent, float lunge_distance, float lunge_speed) {

    const Vector2 movement = direction_to_vector2(ent->direction);

    /*float lunge_distance = 1.0f;
    float lunge_speed = 2.8f;*/
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


void ai_simple_follow_melee_attack(Entity* ent, Entity* target, EntityData* entity_data, MapData* map_data) {

    // calculate next move state
    if (!ent->found_target) {
        if (is_entity_visible(ent, target, map_data, false)) {
            if (!ent->found_target) {
                ent->found_target = true;
            }
        }
    }

    // still no target found
    if (!ent->found_target) {
        ent->state = SKIP_TURN;
        return;
    }

    PathList path_list = find_path_through_ents(
        ent,
        get_active_position(target),
        false,
        map_data
    );
    if (path_list.unreachable) {
        ent->state = SKIP_TURN;
        return;
    }
    //printf("Path len: %i\n", path_list.length);

    // If a path is found, move towards the target, otherwise attack if adjacent
    if (path_list.length >= 1) {
        Point next_p = path_list.path[path_list.length - 1];
        Vector2 next_v = point_to_vector2(next_p);
        Vector2 movement = Vector2Subtract(next_v, ent->original_position);

        // next pos has ent?
        // consider checking if next tile is wall?
        //bool b = any_entity_exists_on_tile(next_p.x, next_p.y, entity_data, ent, NULL);

        bool b = (any_entity_exists_on_tile(next_p.x, next_p.y, entity_data, ent, NULL)
              || map_data->tiles[next_p.x][next_p.y].reserved);
        if (b) {
            PathList alt_path = find_path_around_ents(
                ent,
                get_active_position(target)/*target->original_position*/,
                false,
                map_data,
                entity_data);

            if (alt_path.unreachable) {
                ent->state = SKIP_TURN;
                return;
            }

            // found valid alternative path
            if (alt_path.length >= 1) {
                bool over = abs(alt_path.length - path_list.length) > 3;
                if (over) {
                    ent->state = SKIP_TURN;
                    return;
                }

                next_p = alt_path.path[alt_path.length - 1];

                if (any_entity_exists_on_tile(next_p.x, next_p.y, entity_data, ent, NULL) ||
                    map_data->tiles[next_p.x][next_p.y].type == TILE_WALL ||
                    map_data->tiles[next_p.x][next_p.y].reserved) {
                    ent->state = SKIP_TURN;
                    // cannot fin an alternative path
                    return;
                }

                next_v = point_to_vector2(next_p);
                movement = Vector2Subtract(next_v, ent->original_position);
                //printf("Entity movement: ");
                //print_vector2(movement);

                printf("Entity move to: ");
                print_vector2(next_v);

                if (ent->state == IDLE)
                    ent->direction = vector_to_direction(movement);
                ent->state = MOVE;
                map_data->tiles[next_p.x][next_p.y].reserved = true;
                return;
            }
            else {
                ent->state = SKIP_TURN;
                return;
            }
        }

        //printf("Entity movement: ");
        //print_vector2(movement);
          printf("Entity move to: ");
          print_vector2(next_v);

        if (ent->state == IDLE)
            ent->direction = vector_to_direction(movement);
        ent->state = MOVE;
        map_data->tiles[next_p.x][next_p.y].reserved = true;
    }
    else {
        // If next to target and it's this entity's turn, attack
        Vector2 movement = Vector2Subtract(target->original_position, ent->original_position);
        if (ent->state == IDLE)
            ent->direction = vector_to_direction(movement);
        ent->state = ATTACK_MELEE;
    }
}

void ai_fantano_teleport_to_same_room_as_player_and_defend(Entity* ent, Entity* player, EntityData* entity_data, MapData* map_data, ItemData* item_data) {

	Vector2 dir_to_player = Vector2Clamp(
		Vector2Subtract(get_active_position(player), ent->original_position),
		(Vector2) {
		-1.0f, -1.0f
	},
		(Vector2) {
		1.0f, 1.0f
	});
	ent->direction = vector_to_direction(dir_to_player);

    if (ent->hp < ent->max_hp) ent->hp = ent->max_hp;
    
    if (player->hp < 50) {
        for (int i = 0; i < entity_data->entity_counter; i++) {
            Entity* e = &entity_data->entities[i];
            if (e == ent) continue;
            if (e == player) continue;
            e->n_turn = e->max_turns;

            apply_damage(e, ent, 999, false, true, entity_data);
            printf("DEFEND!\n");
            ent->state = SKIP_TURN;
        }
        ent->state = SKIP_TURN;
        //return;
    }

    if (player->cur_room == -1) {
        ent->state = SKIP_TURN;
        return;
    }

    // player is in a room
    // pickup random tile in that room
    // fantano not in same room as player

    if (player->cur_room == ent->cur_room) {
        ent->state = SKIP_TURN;
        return;
    }

    int i = player->cur_room;
    Room* rm = &map_data->rooms[i];

    while (true) {
        // find clear tile to tp onto
        int col = GetRandomValue(rm->x, rm->x + rm->cols);
        int row = GetRandomValue(rm->y, rm->y + rm->rows);

        if (any_entity_exists_on_tile(col, row, entity_data, NULL, NULL))
            continue;

        if (item_exists_on_tile(col, row, item_data, NULL))
            continue;

        if (map_data->tiles[col][row].type == TILE_WALL)
            continue;

        // check if block
        bool block = false;
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                if (map_data->tiles[col + x][row + y].type == TILE_CORRIDOR) {
                    block = true;
                    break;
                }
            }
			if (block) break;
        }
        if (block) continue;

        set_entity_position(ent, (Vector2) { col, row });
        ent->state = SKIP_TURN;
        break;
    }
}

void entity_think(Entity* ent, Entity* player, MapData* map_data, EntityData* entity_data, ItemData* item_data, Vector2 grid_mouse_position) {
    // player is usually entity index 0
    if (ent == player) {
        // get player next action
        control_entity(ent, map_data, entity_data, item_data, grid_mouse_position);
    }
    else {
        if (ent->is_swapping) {
            // ent->sync_move = true;
            Vector2 movement = get_tile_infront_entity(ent); 
            //printf("Swap movement: ");
            //print_vector2(Vector2Subtract(movement, ent->original_position));
            printf("Entity swap to: ");
            print_vector2(movement);
            ent->state = MOVE;
            map_data->tiles[(int)movement.x][(int)movement.y].reserved = true;
            printf("Reserved (%i): ", map_data->tiles[(int)movement.x][(int)movement.y].reserved);
            print_vector2(movement);
            return;
        }

        switch (ent->ent_type) {
        case ENT_ZOR:
            ent->state = SKIP_TURN;
            //ai_simple_follow_melee_attack(ent, player, entity_data, map_data);
            break;
        case ENT_FLY: {
            ai_simple_follow_melee_attack(ent, player, entity_data, map_data);
            // ent->state = SKIP_TURN;
            break;
        }
        case ENT_FANTANO: {
            ai_fantano_teleport_to_same_room_as_player_and_defend(ent, player, entity_data, map_data, item_data);
            break;
        }
        default:
            break;
        }
    }
}

void decrement_entity_notif_timer(EntityData* entity_data) {
    for (int i = 0; i < MAX_DAMAGE_POPUPS; i++) {
        if (entity_data->damage_popups[i].notif_timer >= 0.0f) {
            entity_data->damage_popups[i].notif_timer -= GetFrameTime();
        }
    }
}


void init_entity_turn_data(EntityTurn* entity_turn_data) {
    for (int i = 0; i < MAX_ENTITY_TURNS; i++) {
        entity_turn_data[i] = (EntityTurn){NULL, 0, false};
    }
}

void process_initial_entity_think(GameStateInfo* gsi, EntityData* entity_data, ItemData* item_data, MapData* map_data) {

    if (entity_data->finished_thinking) return;
    Entity* zor = &entity_data->entities[0];
    for (int i = 0; i < entity_data->entity_counter; i++) {
        Entity* ent = &entity_data->entities[i];
        entity_think(ent, zor, map_data, entity_data, item_data, gsi->grid_mouse_position);
    }
    entity_data->finished_thinking = true;
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