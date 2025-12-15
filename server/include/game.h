#ifndef GAME_H
#define GAME_H

#include <vector>
#include "cards.h"
#include <map>

class Game
{
private:
    int start_balance;
    bool isWinner;
    std::vector<int> table_players;
    std::vector<Card *> cards;
    std::map<int, int> player_balance;

public:
    Game(std::vector<int> players, int start_balance, bool isWinner);
    ~Game();

    void run();
};

#endif