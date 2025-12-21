#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class GamePhase
{
    WAITING,
    PREFLOP,
    FLOP,
    TURN,
    RIVER,
    SHOWDOWN
};

enum class PlayerStatus
{
    ACTIVE,
    FOLDED,
    ALL_IN
};

#endif
