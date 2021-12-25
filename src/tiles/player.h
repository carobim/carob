#ifndef SRC_TILES_PLAYER_H_
#define SRC_TILES_PLAYER_H_

#include "tiles/character.h"
#include "tiles/vec.h"
#include "util/compiler.h"

struct Exit;

class Player : public Character {
 public:
    static Player&
    instance() noexcept;

    Player() noexcept;
    void
    destroy() noexcept;

    //! Smooth continuous movement.
    void
    startMovement(ivec2 delta) noexcept;
    void
    stopMovement(ivec2 delta) noexcept;

    //! Move the player by dx, dy. Not guaranteed to be smooth if called
    //! on each update().
    void
    moveByTile(ivec2 delta) noexcept;

    //! Try to use an object in front of the player.
    void
    useTile() noexcept;

    void
    setFrozen(bool b) noexcept;

 protected:
    void
    arrived() noexcept;

    void
    takeExit(Exit& exit) noexcept;

 private:
    //! Stores intent to move continuously in some direction.
    ivec2 velocity;

    //! Stack storing depressed keyboard keys in the form of movement vectors.
    ivec2 movements[8];
    Size numMovements;
};

#endif  // SRC_TILES_PLAYER_H_
