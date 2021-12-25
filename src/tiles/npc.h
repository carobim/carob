#ifndef SRC_TILES_NPC_H_
#define SRC_TILES_NPC_H_

#include "tiles/character.h"
#include "util/compiler.h"

class NPC : public Character {
 protected:
    void
    arrived() noexcept;
};

#endif  // SRC_TILES_NPC_H_
