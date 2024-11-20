#include "systems/entity_processing.h"

#include "dungeon.h"


static void process_entity_state(Entity* ent, EntityData* entity_data) {
    // switch (ent->state) {
    // case IDLE: {
    //     // not really supposed to trigger here
    //     //reset_entity_state(ent, false);
    //     break;
    // }
    // case SKIP_TURN: {
    //     reset_entity_state(ent, true);
    //     break;
    // }
    // case MOVE: {
    //     move_entity_forward(ent);
    //     break;
    // }
    // case ATTACK_MELEE: {
    //     if (ent->attack_delay < ATTACK_DELAY_DURATION) {
    //         if (ent->animation.cur_frame != 0)
    //             ent->animation.cur_frame = 0;
    //         ent->attack_delay += GetFrameTime();
    //         break;
    //     }
    //     process_attack(ent, entity_data);
    //     switch (ent->ent_type) {
    //     default:
    //         // lunge_entity(ent, 1.0f, 2.5f);
    //         break;
    //     case ENT_ZOR:
    //         // lunge_entity(ent, 1.2f, 2.9f);
    //         break;
    //     case ENT_FLY:
    //         lunge_entity(ent, 1.4f, 3.0f);
    //         break;
    //     }
    //     break;
    // }
    // default:
    //     break;
    // }
}



