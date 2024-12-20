#pragma once

#include <stdio.h>

#include "main.h"

//#define LOAD_FANTANO_TEXTURE() (LoadTexture("dependencies/res/fantano/fantano_idle.png"))
//#define LOAD_CYHAR_TEXTURE() (LoadTexture("dependencies/res/cyhar/cyhar_idle.png"))

Entity default_ent_zor() {

	Animation anim = (Animation){
		.n_frames = 20
	};
	
	Entity ent = (Entity){
		.ent_type = ENT_ZOR,
		.position = (Vector2){0.0f, 0.0f},
		.texture = LoadTexture("dependencies/res/zor/zor_spritesheet.png"),

		.animation = anim,
		.state = IDLE,
		.direction = GetRandomValue(0,7),
		.lunge_progress = 0.0f,
		.cur_move_anim_extra_frame = 0,

		.inventory = { 0 },
		.inventory_size = 3,
		.inventory_item_count = 0,
		.equipped_item_index = 0,
		.can_pickup = true,

		.original_position = (Vector2){0.0f, 0.0f},
		.can_swap_positions = false,

		.n_turn = 0,
		.max_turns = 1,
		.sync_move = false,

		.attack_damage_given = false,

		.cur_room = -1,
		.found_target = false,

		.atk = 3,
		.max_hp = 100,
		.hp = 100
	};
	return ent;
}

Entity create_fly_entity() {
	Animation anim = (Animation){
		.n_frames = 20
	};

	Entity ent = (Entity){
		.ent_type = ENT_FLY,
		.position = (Vector2){0.0f, 0.0f},
		.texture = LoadTexture("dependencies/res/entities/fly.png"),

		.animation = anim,
		.state = IDLE,
		.direction = GetRandomValue(0,7),
		.lunge_progress = 0.0f,
		.cur_move_anim_extra_frame = 0,

		.inventory = { 0 },
		.inventory_size = 1,
		.inventory_item_count = 0,
		.equipped_item_index = 0,
		.can_pickup = false,

		.original_position = (Vector2){0.0f, 0.0f},
		.can_swap_positions = true,

		.n_turn = 0,
		.max_turns = 1,
		.sync_move = false,

		.attack_damage_given = false,

		.cur_room = -1,
		.found_target = false,

		.atk = 9, // 9
		.max_hp = 11,
		.hp = 11
	};
	return ent;
}

Entity create_fantano_entity() {
	Animation anim = (Animation){
		.n_frames = 20,
		.max_frame_time = 0.017f,
	};

	Entity ent = (Entity){
		.ent_type = ENT_FANTANO,
		.position = (Vector2){0.0f, 0.0f},
		.texture = LoadTexture("dependencies/res/fantano/fantano_idle.png"),

		.animation = anim,
		.state = IDLE,
		.direction = GetRandomValue(0,7),
		.lunge_progress = 0.0f,
		.cur_move_anim_extra_frame = 0,

		.inventory = { 0 },
		.inventory_size = 1,
		.inventory_item_count = 0,
		.equipped_item_index = 0,
		.can_pickup = false,

		.original_position = (Vector2){0.0f, 0.0f},
		.can_swap_positions = true,

		.n_turn = 0,
		.max_turns = 1,
		.sync_move = false,

		.attack_damage_given = false,

		.cur_room = -1,
		.found_target = false,

		.atk = 999,
		.max_hp = 999,
		.hp = 999
	};
	return ent;
}
