#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "GameState.h"

class Player
{
    int id;
    int balance;

    std::string card1;
    std::string card2;

    int currentBet;
    PlayerStatus status;

public:
    Player(int id);
    ~Player();
    int getId() const;

    int getBalance() const;
    void setBalance(int amount);

    const std::string &getCard1() const;
    void setCard1(const std::string &card);

    const std::string &getCard2() const;
    void setCard2(const std::string &card);

    int getCurrentBet() const;
    void setCurrentBet(int amount);

    PlayerStatus getStatus() const;
    void setStatus(PlayerStatus newStatus);

    void resetForNewRound();
};

#endif