#include "core/npc.h"
#include "util/compiler.h"

void
NPC::arrived() noexcept {
    Entity::arrived();

    if (destExit) {
        moving = false;  // Prevent time rollover check in
                         // Entity::moveTowardDestination().
        destroy();
    }
}
