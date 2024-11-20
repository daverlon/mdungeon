#include "systems/animation.h"

void update_animation_state(Entity* ent) {
    switch (ent->ent_type) {
    case ENT_ZOR: {
        switch (ent->state) {
        case IDLE: {

            ent->animation.max_frame_time = 0.020f;
            switch_to_idle_y_offset(ent);

            break;
        }
        case MOVE: {
            ent->animation.max_frame_time = 0.030f;
            ent->animation.y_offset = 2048;
            break;
        }
        case ATTACK_MELEE: {
            if (ent->attack_delay > ATTACK_DELAY_DURATION) {
                ent->animation.max_frame_time = 0.023f;
                ent->animation.y_offset = 2048 + 2048;
            }

            /*ent->animation.max_frame_time = 0.017f;
            ent->animation.y_offset = 2048 + 2048;*/
            //ent->cur_move_anim_extra_frame = 0;
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    case ENT_FANTANO: {
        switch (ent->state) {
        case IDLE: {
            ent->animation.max_frame_time = 0.017f;
            switch_to_idle_y_offset(ent);

            break;
        }
        case ATTACK_MELEE: {
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    case ENT_CYHAR: {
        switch (ent->state) {
        case IDLE: {
            ent->animation.max_frame_time = 0.019f;
            switch_to_idle_y_offset(ent);

            break;
        }
        case ATTACK_MELEE: {
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    case ENT_FLY: {
        switch (ent->state) {
        case IDLE: {
            ent->animation.max_frame_time = 0.037f;
            switch_to_idle_y_offset(ent);

            break;
        }
        case ATTACK_MELEE: {
            if (ent->attack_delay > ATTACK_DELAY_DURATION) {
                ent->animation.max_frame_time = 0.010f;
                ent->animation.y_offset = 2048;
            }
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    default:
        break;
    }

    if (ent->state == MOVE) {
        ent->cur_move_anim_extra_frame = 0;
    }
}