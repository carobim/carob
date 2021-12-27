#include "tiles/npc.h"
#include "util/compiler.h"

void
NPC::arrived() noexcept {
    Character::arrived();

    if (destExit) {
        moving = false;  // Prevent time rollover check in
                         // Entity::moveTowardDestination().
        destroy();
    }
}
