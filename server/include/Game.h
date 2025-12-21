#ifndef GAME_H
#define GAME_H

#include <vector>
#include <map>
#include <iostream>
#include "./utils/Logger.h"
#include <string>
#include "Player.h"
#include "Board.h"
#include <mutex>
#include <queue>

class Server;

struct PlayerAction
{
    int playerId;
    std::string command;
    int amount;
};

class Game
{
private:
    Server *server;
    Board gameBoard;
    Logger logger;

    std::mutex gameMutex;
    std::queue<PlayerAction> actionQueue;

    int start_balance;
    bool isWinner;
    std::vector<Player *> table_players;
    std::map<Player *, int> player_balance;
    std::vector<std::string> deck;
    std::map<std::string, int> deckIndices;

    void bettingPhase(std::string phaseName);
    void processQueue();
    void determineWinner();

public:
    Game(std::vector<Player *> players, int start_balance);
    ~Game();

    void playerBet(Player *p, int amount);

    void run();
    void setServer(Server *srv)
    {
        this->server = srv;
    }
    void queueAction(int playerId, const std::string &command, int amount);
};

#endif